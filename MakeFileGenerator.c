#include <stdio.h>
#include <openssl/md5.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>


#define TRUE 1
#define FALSE 0
#define DEFAULT_NUMBER_FILES_IN_DIR 10

#define FOLDER_CONFIG_NAME ".mkfileGen"
#define MD5SUM_LOG_FILE ".md5log"
#define MAX_FOLDER_PATH_SIZE 2048

typedef struct fileInformations{
  unsigned char* name;
  unsigned char* MD5Sum;
}fileInformations;

typedef struct filesOfDirectory{
  fileInformations* arrayOfFiles;
  unsigned long numberOfFiles;
  const char* directoryPath;
}filesOfDirectory;

unsigned char* getMD5OfFile(const char* filepath){
  unsigned char* c = (unsigned char*) malloc(MD5_DIGEST_LENGTH);

  int i;
  FILE *inFile = fopen (filepath, "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];
  if (inFile == NULL) {
      printf ("%s can't be opened.\n", filepath);
      return 0;
  }
  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
      MD5_Update (&mdContext, data, bytes);
  MD5_Final (c,&mdContext);
  for(i = 0; i < MD5_DIGEST_LENGTH; i++){
    //printf("%02x", c[i]);
  }
  //printf (" %s\n", filepath);
  fclose (inFile);
  return c;
}

//TODO handle realloc
/*Returns a struct of filesOfDirectory, for current dir */
filesOfDirectory getCFilesInDirectory(const char* dirpath){
  DIR *d;
  struct dirent *dir;
  long numberOfFiles = 0;
  size_t defaultSizeAllocatedForArrayOfFiles = sizeof(fileInformations) * DEFAULT_NUMBER_FILES_IN_DIR;
  filesOfDirectory files = { NULL , numberOfFiles, dirpath};
  memset(&files.arrayOfFiles, '\0', sizeof(files.arrayOfFiles));
  files.arrayOfFiles = malloc(defaultSizeAllocatedForArrayOfFiles);
  d = opendir(dirpath);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      char * currFile = dir->d_name;
      if(strlen(currFile) >= 3){
        files.arrayOfFiles[numberOfFiles].name = (unsigned char *)currFile;
        files.arrayOfFiles[numberOfFiles].MD5Sum = getMD5OfFile(currFile);
        numberOfFiles++;
        if(numberOfFiles % DEFAULT_NUMBER_FILES_IN_DIR == 0){
          files.arrayOfFiles = realloc(files.arrayOfFiles,
            (sizeof(fileInformations) * numberOfFiles)
            + defaultSizeAllocatedForArrayOfFiles);
        }
      }
    }
    closedir(d);
  }
  files.numberOfFiles = numberOfFiles;
  return files;
}

void printMD5(unsigned char* MD5){
  for(int i = 0; i < MD5_DIGEST_LENGTH; i++){
    printf("%02x", MD5[i]);
  }
  putchar('\n');
}


/*Returns boolean if file exists or not*/
int isFileExisting(const char * filePath){
  if(access(filePath, F_OK) != -1) {
    return TRUE;
  } else {
    return FALSE;
  }
}


/*Returns boolean to says if you need to remake*/
int checkMD5Sums(filesOfDirectory files){
  int needToRemake = FALSE;
  char folderPath[2048] = ".mkfileGen";
  struct stat sb;
  int folderDoesntExists = (stat(folderPath, &sb) == -1);
  if(folderDoesntExists) {
    mkdir(folderPath, 0700); //Creating dir
    needToRemake = TRUE;
  }



  FILE* fileToCreate;


  if(isFileExisting(strcat(strcat(folderPath, "/"), MD5SUM_LOG_FILE))){
    //compare sums
    //fwrite(fileToCreate, );
  } else {
    fileToCreate = fopen(folderPath, "w+");
    needToRemake = TRUE;
  }

  return needToRemake;
}


int main(int argc, char const *argv[]) {
  //getMD5OfFile("text.txt");
  filesOfDirectory filesInCurrDir = getCFilesInDirectory(".");
  printf("name %s\n", filesInCurrDir.arrayOfFiles[0].name);
  printf("Number of files = %ld\n", filesInCurrDir.numberOfFiles);
  printMD5(filesInCurrDir.arrayOfFiles[0].MD5Sum);
  for(int i = 0; i < filesInCurrDir.numberOfFiles; i++){

    if(i==0)checkMD5Sums(filesInCurrDir);
    //Create .h
  }

  return 0;
}
