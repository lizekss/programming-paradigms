using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>
#include <iostream>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}


bool imdb::getCredits(const string& player, vector<film>& films) const { 
  int numActors = *(int*)actorFile;
  void *actorRecord = binarySearch(actorFile, (void*)player.c_str(), actorCompare, (int*)actorFile + 1, (int*)actorFile + numActors);

  if (actorRecord == NULL)
    return false;

  // Advance to the end of the name string, accounting for the padding
  int b = player.length() + 1;
  if (b % 2 != 0) b++;
  
  short numFilms = *(short*)((char*)actorRecord + b);

  // Advance past the short variable, accounting for the padding
  b += 2;
  if (b % 4 != 0) b += 2;
  
  int *filmOffsets = (int*)((char*)actorRecord + b);
  
  for (int i = 0; i < numFilms; i++) {
    film f = constructFilm((char*)movieFile + filmOffsets[i]);
    films.push_back(f);
  }

  return true; 
}


bool imdb::getCast(const film& movie, vector<string>& players) const {
  int numMovies = *(int*)movieFile;
  void *movieRecord = binarySearch(movieFile, (void*)&movie, filmCompare, (int*)movieFile + 1, (int*)movieFile + numMovies);

  if (movieRecord == NULL)
    return false;

  int b = movie.title.length() + 2;
  if (b % 2 != 0) b++;

  short numActors = *(short*)((char*)movieRecord + b);

  b += 2;
  if (b % 4 != 0) b += 2;

  int *actorOffsets = (int*)((char*)movieRecord + b);

  for (int i = 0; i < numActors; i++) {
    char *c_actor = (char*)actorFile + actorOffsets[i];
    string actor(c_actor);
    players.push_back(actor);
  }

  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

void * imdb::binarySearch(const void *file, void *target, int (*compareFn)(void*, void*, const imdb *obj), int *a, int *b) const{
  int *mid = a + (b - a) / 2;

  while (a <= b) {
    void *midRecord = (char*)file + *mid;
    int comparison = compareFn(midRecord, target, this);

    if (comparison == 0) {
      return midRecord;
    }
    else if (comparison < 0)
      a = mid + 1;
    
    else if (comparison > 0)
      b = mid - 1;

    mid = a + (b - a) / 2;
  } 

  return NULL;
}

film imdb::constructFilm(void *movieRecord) const {
  film f;
  char *c_title = (char *)movieRecord;
  string title(c_title);
  f.title = title;

  int delta = *(char*)((char*)movieRecord + title.length() + 1);
  f.year = 1900 + delta;
  return f;
}

int imdb::actorCompare(void *x, void *y, const imdb *obj) {
  return strcmp((const char *)x, (const char *)y);
}

int imdb::filmCompare(void *recordPtr, void *targetFilmPtr, const imdb *obj) {
  film lhs = obj->constructFilm(recordPtr);
  film rhs = *(film*)targetFilmPtr;
  
  if (lhs < rhs) return -1;
  if (lhs == rhs) return 0;
  else return 1;
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
