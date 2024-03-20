#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "branch.h"

/*
 * deposit money into an account */
 

BranchID
getBranchID(AccountNumber accountNum)
{
  Y;
  return (BranchID) (accountNum >> 32);
}

int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  pthread_mutex_lock(&account->lock);
  pthread_mutex_lock(&(bank->branches[getBranchID(accountNum)].lock));
  Account_Adjust(bank, account, amount, 1);
  pthread_mutex_unlock(&(bank->branches[getBranchID(accountNum)].lock)); 
  pthread_mutex_unlock(&account->lock);
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account */
 
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  pthread_mutex_lock(&account->lock);
  pthread_mutex_lock(&(bank->branches[getBranchID(accountNum)].lock));

  if (amount > Account_Balance(account)) {
    pthread_mutex_unlock(&account->lock);
    pthread_mutex_unlock(&(bank->branches[getBranchID(accountNum)].lock));

    return ERROR_INSUFFICIENT_FUNDS;
  }
  
  Account_Adjust(bank, account, -amount, 1);
  pthread_mutex_unlock(&account->lock);
  pthread_mutex_unlock(&(bank->branches[getBranchID(accountNum)].lock));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account */
 
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  
  if (srcAccountNum < dstAccountNum) {
    pthread_mutex_lock(&srcAccount->lock);
    pthread_mutex_lock(&dstAccount->lock);
  } else if (srcAccountNum > dstAccountNum) {
    pthread_mutex_lock(&dstAccount->lock);
    pthread_mutex_lock(&srcAccount->lock);
  } else {
    return ERROR_SUCCESS;
  }

  if (amount > Account_Balance(srcAccount)) {
    pthread_mutex_unlock(&dstAccount->lock);
    pthread_mutex_unlock(&srcAccount->lock);
    return ERROR_INSUFFICIENT_FUNDS;
  } 
  
  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0. */
   
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  BranchID srcId, dstId;
  
  
  if (updateBranch) {
    srcId = getBranchID(srcAccountNum);
    dstId = getBranchID(dstAccountNum);

    if (srcId < dstId) {
      pthread_mutex_lock(&(bank->branches[srcId].lock));
      pthread_mutex_lock(&(bank->branches[dstId].lock));
    } else {
      pthread_mutex_lock(&(bank->branches[dstId].lock));
      pthread_mutex_lock(&(bank->branches[srcId].lock));
    }

  }


  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);
 
  pthread_mutex_unlock(&dstAccount->lock);
  pthread_mutex_unlock(&srcAccount->lock);
  
  if (updateBranch) {
    pthread_mutex_unlock(&(bank->branches[srcId].lock));
    pthread_mutex_unlock(&(bank->branches[dstId].lock));
  } 

  return ERROR_SUCCESS;
} 