#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>

void parseOptions(int argc, char** argv, char* options)
{
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i], "variant") == 0){
			options[0] = '1';
			break;
		}
		if(strcmp(argv[i], "list") == 0){
			options[1] = '1';
			continue;
		}
		if(strcmp(argv[i], "parse") == 0){
			options[2] = '1';
			continue;
		}
		if(strcmp(argv[i], "extract") == 0)
			options[3] = '1';
		if(strcmp(argv[i], "findall") == 0)
			options[4] = '1';
		if(strcmp(argv[i], "recursive") == 0)
			options[5] = '1';
		if(strncmp(argv[i], "path=", 5) == 0)
			options[6] = i + '0';
		if(strncmp(argv[i], "name_ends_with=", 15) == 0)
			options[7] = i + '0';
		if(strncmp(argv[i], "permissions=", 12) == 0)
			options[8] = i + '0';
		if(strncmp(argv[i], "section=", 8) == 0)
			options[9] = i + '0';
		 if(strncmp(argv[i], "line=", 5) == 0)
			options[10] = i + '0';
	}
}

void parsePermissions(struct stat statBuff, char* perm){
	perm[0] = (statBuff.st_mode & S_IRUSR) ? 'r' : '-';
	perm[1] = (statBuff.st_mode & S_IWUSR) ? 'w' : '-';
	perm[2] = (statBuff.st_mode & S_IXUSR) ? 'x' : '-';
	perm[3] = (statBuff.st_mode & S_IRGRP) ? 'r' : '-';
	perm[4] = (statBuff.st_mode & S_IWGRP) ? 'w' : '-';
	perm[5] = (statBuff.st_mode & S_IXGRP) ? 'x' : '-';
	perm[6] = (statBuff.st_mode & S_IROTH) ? 'r' : '-';
	perm[7] = (statBuff.st_mode & S_IWOTH) ? 'w' : '-';
	perm[8] = (statBuff.st_mode & S_IXOTH) ? 'x' : '-';
}

void listFolderContents(const char* dirPath, bool isRecursive, const char* nameEnd, const char* permissions, bool checkName, bool checkPerm, int depth)
{
	DIR *dir = opendir(dirPath);
	struct dirent *entry = NULL;
	char fullPath[512];
	char filePerm[10];
	char fileName[100];
	struct stat statBuff;
	if(dir == NULL){
		printf("ERROR\ninvalid directory path\n");
		return;
	}
	else if(depth == 0)
		printf("SUCCESS\n");
	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		
		snprintf(fullPath, 512, "%s/%s", dirPath, entry->d_name);
		
		if(lstat(fullPath, &statBuff) != 0) continue;
		
		if(checkPerm) parsePermissions(statBuff, filePerm);
		else strcpy(filePerm, "");
		
		if(checkName)
			strcpy(fileName, entry->d_name);
		else strcpy(fileName, "");
		
		
		if(strcmp(fileName + (strlen(fileName) - strlen(nameEnd)), nameEnd) == 0 && strcmp(permissions, filePerm) == 0){
			if(S_ISDIR(statBuff.st_mode))
				if(isRecursive == true)
					listFolderContents(fullPath, isRecursive, nameEnd, permissions, checkName, checkPerm, depth + 1);
			printf("%s\n", fullPath);
		}
	}
	closedir(dir);
}

typedef struct{
	char name[19];
	int type;
	int offset;
	int size;
}sectionHeader;

typedef struct{
	int version;
	char noOfSections;
	sectionHeader* sectHeaders;
	short size;
	char magic[3];
}headerInfoT;

const char* readHeader(const char* dirPath, bool* success, headerInfoT* headerInfo)
{
	*success = false;
	int fd = -1;
	fd = open(dirPath, O_RDONLY);
	
	if(fd == -1){
		//printf("Error\nwrong path\n");
		return "Error\nwrong path\n";
	}
	
	lseek(fd, -4, SEEK_END);
	read(fd, &headerInfo->size, sizeof(short));
	read(fd, &headerInfo->magic, 2 * sizeof(char));
	if(strcmp(headerInfo->magic, "nd") != 0){
		//printf("ERROR\nwrong magic\n");
		return "ERROR\n wrong magic\n";
	}
	lseek(fd, -(headerInfo->size), SEEK_END);
	read(fd, &headerInfo->version, sizeof(int));
	if(headerInfo->version < 42 || headerInfo->version > 155){
		//printf("ERROR\nwrong version\n");
		return "ERROR\nwrong version\n";
	}
	read(fd, &headerInfo->noOfSections, sizeof(char));
	if(headerInfo->noOfSections < 7 || headerInfo->noOfSections > 15){
		//printf("ERROR\nwrong sect_nr\n");
		return "ERROR\nwrong sect_nr\n";
	}
	headerInfo->sectHeaders = (sectionHeader*)malloc(headerInfo->noOfSections * sizeof(sectionHeader));
	for(int i=0;i<headerInfo->noOfSections;i++){
		read(fd, &headerInfo->sectHeaders[i].name, 18* sizeof(char));
		read(fd, &headerInfo->sectHeaders[i].type,  sizeof(char));
		if(headerInfo->sectHeaders[i].type != 32 && headerInfo->sectHeaders[i].type != 86 && headerInfo->sectHeaders[i].type != 35 && headerInfo->sectHeaders[i].type != 72 && headerInfo->sectHeaders[i].type != 21 && headerInfo->sectHeaders[i].type != 67 && headerInfo->sectHeaders[i].type != 15){
			//printf("ERROR\nwrong sect_types\n");
			return "ERROR\nwrong sect_types\n";
		}
		read(fd, &headerInfo->sectHeaders[i].offset, sizeof(int));
		read(fd, &headerInfo->sectHeaders[i].size, sizeof(int));
	}
	*success = true;
	close(fd);
	return "";
}

void extractCommand(const char* path, int offset, int line, int size){
	int fd = -1;
	char cBuff;
	int nrOfLines = 0;
	
	fd = open(path, O_RDONLY);
	
	if(fd == -1){
		printf("ERROR\ninvalid path\n");
		return;
	}
	
	printf("SUCCESS\n");
	
	lseek(fd, offset, SEEK_SET);
	
	int i;
	for(i=0;i<size;i++){
		read(fd, &cBuff, sizeof(char));
		if(cBuff == '\n')
			nrOfLines++;
	}
	if(cBuff != '\n')
		nrOfLines++;
	
	if(line > nrOfLines){
		printf("ERROR\ninvalid line\n");
		return;
	}

	lseek(fd, offset, SEEK_SET);
	
	while(nrOfLines!=line){
		read(fd, &cBuff, sizeof(char));
		if(cBuff == '\n')
			nrOfLines--;
	}
	
	i=0;
	do{
		read(fd, &cBuff, sizeof(char));
		printf("%c", cBuff);
		i++;
	}while(cBuff != '\n' && i < size);
	
	//printf("%d\n", nrOfLines);
	
	close(fd);
}

void findAllCommand(const char* dirPath, int depth){
	DIR *dir = opendir(dirPath);
	struct dirent *entry = NULL;
	char fullPath[512];
	struct stat statBuff;
	bool wasSuccess;
	
	if(dir == NULL){
		printf("ERROR\ninvalid directory path\n");
		return;
	}
	else if(depth == 0)
		printf("SUCCESS\n");
	
	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
			
		snprintf(fullPath, 512, "%s/%s", dirPath, entry->d_name);
		
		if(lstat(fullPath, &statBuff) != 0) continue;
		
		if(S_ISDIR(statBuff.st_mode))
			findAllCommand(fullPath, depth + 1);
		else{
			headerInfoT* headerInfo = (headerInfoT*)malloc(sizeof(headerInfoT));
		
			readHeader(fullPath, &wasSuccess, headerInfo);
			if(wasSuccess)
				for(int i=0;i<headerInfo->noOfSections;i++)
					if(headerInfo->sectHeaders[i].type == 72){
						printf("%s\n", fullPath);
						break;
					}
			free(headerInfo->sectHeaders);
			free(headerInfo);
		}
	}
	closedir(dir);
}

int main(int argc, char**argv)
{
	if(argc >= 2){
		char* options = (char*)malloc(12 * sizeof(char));
		strcpy(options, "00000000000\0");
		parseOptions(argc, argv, options);
		
		if(options[0] == '1'){
			printf("34300\n");
			goto jumpToEnd;
		}
		if(options[1] == '1'){
			if(options[7] != '0' && options[8] != '0')
				listFolderContents(argv[options[6] - '0'] + 5, options[5] - '0', argv[options[7] - '0'] + 15, argv[options[8] - '0'] + 12, true, true, 0);
			if(options[7] != '0' && options[8] == '0')
				listFolderContents(argv[options[6] - '0'] + 5, options[5] - '0', argv[options[7] - '0'] + 15, "", true, false, 0);
			if(options[7] == '0' && options[8] != '0')
				listFolderContents(argv[options[6] - '0'] + 5, options[5] - '0', "", argv[options[8] - '0'] + 12, false, true, 0);
			if(options[7] == '0' && options[8] == '0')
				listFolderContents(argv[options[6] - '0'] + 5, options[5] - '0', "", "", false, false, 0);
			goto jumpToEnd;
		}
		
		if(options[4] == '1'){
			findAllCommand(argv[options[6] - '0'] + 5, 0);
			goto jumpToEnd;
		}
		
		headerInfoT* header = (headerInfoT*)malloc(sizeof(headerInfoT));
		bool wasSuccess;
			
		const char* message = readHeader(argv[options[6] - '0'] + 5, &wasSuccess, header);
		if(options[2] == '1'){
			if(wasSuccess){
				printf("SUCCESS\nversion=%d\nnr_sections=%d\n", header->version, header->noOfSections);
				for(int i=0;i<header->noOfSections;i++){
					printf("section%d: %s %d %d\n", i+1, header->sectHeaders[i].name, header->sectHeaders[i].type, header->sectHeaders[i].size);
				}
			}
			else{
				printf("%s", message);
			}
		}
		if(options[3] == '1'){
			if(header->noOfSections < atoi(argv[options[9] - '0'] + 8))
				printf("ERROR\ninvalid section\n");
			else
				extractCommand(argv[options[6] - '0'] + 5, header->sectHeaders[atoi(argv[options[9] - '0'] + 8) - 1].offset, atoi(argv[options[10] - '0'] + 5), header->sectHeaders[atoi(argv[options[9] - '0'] + 8) - 1].size);
		}
		
		free(header->sectHeaders);
		free(header);
			
		jumpToEnd:
		free(options);
	}
	return 0;
}
