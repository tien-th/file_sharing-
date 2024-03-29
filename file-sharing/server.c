#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include "communication_code.h"
#include "linked_list.h"
#include <time.h>
#include <pthread.h>

#define BUFF_SIZE 100
singleList groups, files, users, requests;

// fix bug 

void checkGroup(singleList groups){
	// print all information of groups

	printf ("\n@@@@@@@@@@@@@@@@@@@@@@ Check All groups @@@@@@@@@@@@@@@@@@@@@@\n");

	groups.cur = groups.root;
	while (groups.cur != NULL)
	{	
		printf("\n-------------------------------\n");
		printf("group name: %s\n", ((group_struct*)groups.cur->element)->group_name);
		printf("owner: %s\n", ((group_struct*)groups.cur->element)->owner);
		printf("number of members: %d\n", ((group_struct*)groups.cur->element)->number_of_members);
		//  print all members of group
		singleList members; 
		createSingleList(&members);
		members = ((group_struct*)groups.cur->element)->members;
		members.cur = members.root;
		while(members.cur!=NULL){
			printf("member: %s\n", ((simple_user_struct*)members.cur->element)->user_name);
			members.cur = members.cur->next;
		}
		
		printf("number of files: %d\n", ((group_struct*)groups.cur->element)->number_of_files);

		//  print all files of group
		singleList files;
		createSingleList(&files);
		files = ((group_struct*)groups.cur->element)->files;
		files.cur = files.root;
		while(files.cur!=NULL){
			printf("file: %s\n", ((simple_file_struct*)files.cur->element)->file_name);
			files.cur = files.cur->next;
		}

		printf("\n-------------------------------\n");
		groups.cur = groups.cur->next;
	}
}

void checkFile(singleList files){
	// print all information of files
	files.cur = files.root;
	while (files.cur != NULL)
	{	
		printf("\n-------------------------------\n");
		printf("file name: %s\n", ((file_struct*)files.cur->element)->name);
		printf("owner: %s\n", ((file_struct*)files.cur->element)->owner);
		printf("group: %s\n", ((file_struct*)files.cur->element)->group);
		printf("uploaded at: %s\n", ((file_struct*)files.cur->element)->uploaded_at);
		printf("downloaded times: %d\n", ((file_struct*)files.cur->element)->downloaded_times);
		printf("\n-------------------------------\n");
		files.cur = files.cur->next;
	}
}


void checkUser(singleList users){
	// print all information of users
	printf ("\n@@@@@@@@@@@@@@@@@@@@@@ Check All users @@@@@@@@@@@@@@@@@@@@@@\n");
	users.cur = users.root;
	while (users.cur != NULL)
	{	
		printf("\n-------------------------------\n");
		printf("user name: %s\n", ((user_struct*)users.cur->element)->user_name);
		printf("password: %s\n", ((user_struct*)users.cur->element)->password);
		printf("status: %d\n", ((user_struct*)users.cur->element)->status);
		printf("count group: %d\n", ((user_struct*)users.cur->element)->count_group);

		//  print all joined groups of user
		singleList joined_groups;
		createSingleList(&joined_groups);
		joined_groups = ((user_struct*)users.cur->element)->joined_groups;
		joined_groups.cur = joined_groups.root;
		while(joined_groups.cur!=NULL){
			printf("joined group: %s\n", ((simple_group_struct*)joined_groups.cur->element)->group_name);
			joined_groups.cur = joined_groups.cur->next;
		}

		printf("\n-------------------------------\n");
		users.cur = users.cur->next;
	}
}

void checkRequest(singleList requests){
	// print all information of requests

	printf ("\n@@@@@@@@@@@@@@@@@@@@@@ Check All requests @@@@@@@@@@@@@@@@@@@@@@\n");

	requests.cur = requests.root;
	while (requests.cur != NULL)
	{	
		printf("\n-------------------------------\n");
		printf("group name: %s\n", ((request_struct*)requests.cur->element)->group_name);
		printf("user name: %s\n", ((request_struct*)requests.cur->element)->user_name);
		printf("request from user: %d\n", ((request_struct*)requests.cur->element)->request_from_user);
		printf("\n-------------------------------\n");
		requests.cur = requests.cur->next;
	}
}


char *getGroupOwner(singleList groups, char group_name[50]){
	groups.cur = groups.root;
	while(groups.cur != NULL)
	{
		if(strcmp(((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			return ((group_struct*)groups.cur->element)->owner;
		}
		groups.cur = groups.cur->next;
	}
	return NULL;
}


void deleteRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user);
void convertUserRequestsToString(singleList simple_request, char str[1000]);
int checkRequestExit(singleList requests, char group_name[50], char owner[50], int request_from_user);
int updateRequest(singleList *requests, char group_name[50], char owner[50], int request_from_user);
void readRequestFile(singleList* requests);
void readGroupFile(singleList *groups);
void writeToGroupFile(singleList groups);
void readUserFile(singleList* users);
void readFileFile(singleList *files);
int checkExistence(int type, singleList list, char string[50]);
void* findByName(int type, singleList list, char string[50]);
int addMember(singleList groups, char group_name[50], char username[50]);
int addGroupToJoinedGroups(singleList users, char username[50], char group_name[50]);
singleList unJoinedGroups(singleList groups, singleList users, char username[50]);
void convertSimpleGroupsToString(singleList simple_group, char str[1000]);
void convertSimpleFilesToString(singleList simple_file, char str[1000]);
void convertSimpleUsersToString(singleList simple_user, char str[1000]);
void getBasicInfoOfGroup(singleList groups, char group_name[50], char group_info[200]);
void createGroup(int sock, singleList * groups, user_struct *loginUser);
void sendCode(int sock, int code);
singleList joinedGroups(singleList users, char username[50]);
singleList getAllFilesOfGroup(singleList groups, char group_name[50]);
singleList getAllMembersOfGroup(singleList groups, char group_name[50]);
singleList getFilesOwns(singleList files, char username[50]);
void* SendFileToClient(int new_socket, char fname[50], char group_name[50]);
singleList getFilesCanDelete(singleList files, singleList groups, char group_name[], char username[]);
void deleteFile(singleList *files, singleList groups, char group_name[], char file_name[50]);
int isFileExistInGroup(singleList groups, char group_name[], char file_name[]);
int isOwnerOfGroup(singleList groups, char group_name[], char username[]);
int getAllFilesOfUserInGroup(singleList *files, char group_name[50], char username[50], char all_files[20][50]);
void kickMemberOut(singleList groups, singleList users, char group_name[50], char username[50]);
void signUp(int sock, singleList *users);
int signIn(int sock, singleList users, user_struct **loginUser);
void uploadFile(int sock, user_struct *loginUser);
int receiveUploadedFile(int sock, char filePath[100]);
void * handleThread(void *my_sock);
void sendWithCheck(int sock, char buff[BUFF_SIZE], int length, int option){
	int sendByte = 0;
	sendByte = send(sock, buff, length, option);
	if(sendByte > 0){
		
	}else{
		close(sock);
		pthread_exit(0);
	}
}

int readWithCheck(int sock, char buff[BUFF_SIZE], int length){
	int recvByte = 0;
	// recvByte = read(sock, buff, length);
	recvByte = recv(sock, buff, length, 0);

	if(recvByte > 0){
		return recvByte;
	}else{
		printf("server recv error\n");
		close(sock);
		pthread_exit(0);
	}
}

//==============MAIN==============
int main(int argc, char *argv[]) 
{
	//catch wrong input
	if(argc==1){
		printf("Please input port number\n");
		return 0;
	}
	char *port_number = argv[1];
	int port = atoi(port_number);
	int opt = 1;
	int server_fd, new_socket; 
	struct sockaddr_in address;
	int addrlen = sizeof(address); 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( port ); 
	
	// Forcefully attaching socket to the port 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 


	//============================Start to communicate with client=====================================
	//=================================================================================================
	char buff[100];

	createSingleList(&groups);
	createSingleList(&files);
	createSingleList(&users);
	createSingleList(&requests);
	
	readGroupFile(&groups);
	readFileFile(&files);
	readUserFile(&users);
	readRequestFile(&requests);
	while(1){
		pthread_t tid; 

		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		printf("New request from sockfd = %d.\n",new_socket);	
        pthread_create(&tid, NULL, &handleThread, &new_socket);
    }
	close(server_fd);
	return 0; 
} 



void readGroupFile(singleList *groups){
	
	// clear list
	deleteSingleList(groups);
	FILE *fp;
	fp = fopen("./storage/group.txt","r");
	char str_tmp[100];

	
	while (1)
	{	
		char c = fgetc(fp);
    	if (c != EOF){
			int res = fseek( fp, -1, SEEK_CUR );
			// fgets (str_tmp, 100, fp);
		}else
        	break;
		//====================initialize============================================
		group_struct *group_element = (group_struct*) malloc(sizeof(group_struct));
		singleList members;
		createSingleList(&members);
		singleList files;
		createSingleList(&files);

		

		//======================end initialize======================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(group_element->group_name, str_tmp);
		//=====================read members=========================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(group_element->owner, str_tmp);
		
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		int number_of_members = atoi(str_tmp);
		group_element->number_of_members = number_of_members; // number_of_members
		for(int i = 0; i < number_of_members; i++){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			fgets(str_tmp, 100, fp);
			str_tmp[strlen(str_tmp)-1] = '\0';
			strcpy(member_element->user_name, str_tmp);
			insertEnd(&members, member_element);
		}
		group_element->members = members;
		//===================end read members========================================
		//=====================read files============================================
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		int number_of_files = atoi(str_tmp);
		group_element->number_of_files = number_of_files;
		for(int i = 0; i < number_of_files; i++){
			simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
			fgets (str_tmp, 100, fp);
			if(str_tmp[strlen(str_tmp)-1] == '\n'){
				str_tmp[strlen(str_tmp)-1] = '\0';
			}
			strcpy(file_element->file_name, str_tmp);
			insertEnd(&files, file_element);
		}
		group_element->files = files;
		//=====================end read files=========================================
		insertEnd(groups, group_element); // add group_element to group_list
	}


	fclose(fp);
}

void writeToGroupFile(singleList groups){
	group_struct* group = NULL;
	FILE *fp;
	fp = fopen("./storage/group.txt","w");
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		group = (group_struct*)(groups.cur->element);
		fprintf(fp,"%s\n", group->group_name);
		fprintf(fp,"%s\n", group->owner);
		fprintf(fp,"%d\n", group->number_of_members);
		singleList members;
		createSingleList(&members);
		members = group->members;
		members.cur = members.root;
		while(members.cur!=NULL){
			fprintf(fp,"%s\n",((simple_user_struct*)members.cur->element)->user_name);
			members.cur = members.cur->next;
		}
		fprintf(fp,"%d\n", group->number_of_files);
		singleList files;
		createSingleList(&files);
		files = group->files;
		files.cur = files.root;
		while(files.cur!=NULL){
			fprintf(fp,"%s\n",((simple_file_struct*)files.cur->element)->file_name);
			files.cur = files.cur->next;
		}
		groups.cur = groups.cur->next;
	}
	fclose(fp);
}

void readUserFile(singleList* users){
	char username[50], password[50], group_name[50];
	int status, count_group;
	FILE * f = fopen("./storage/user.txt","r");

	if(f == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		singleList groups;
		createSingleList(&groups);

		char c = fgetc(f);
    	if (c != EOF){
			int res = fseek( f, -1, SEEK_CUR );
		}else{
			//printf("scan: stop\n");
        	break;
		}

		fgets(username, 50, f);
		username[strlen(username) -1 ] = '\0';
		//printf("scan: %s\n", username);

		fgets(password, 50, f);
		password[strlen(password) -1 ] = '\0';
		//printf("scan: %s\n", password);

		fscanf(f,"%d\n", &status);

		if(fscanf(f, "%d\n", &count_group) > 0){
			for(int i = 0; i < count_group; i++){
				// fscanf(f, "%s\n", group_name);
				fgets(group_name, 50, f);
				group_name[strlen(group_name) -1]  = '\0';
				insertEnd(&groups, strdup(group_name));
			}
		}

		user_struct *user = (user_struct*)malloc(sizeof(user_struct));
		strcpy(user->user_name, username);
		strcpy(user->password, password);
		user->status = status;
		user->joined_groups = groups;
		user->count_group = count_group;

		insertEnd(users, user);

	}
    fclose(f);
	// printf("user: \n ") ; checkUser(*users);

}

void convertUserRequestsToString(singleList requests, char str[1000]){
	str[0] = '\0';
	requests.cur = requests.root;
	while(requests.cur != NULL)
  	{
		strcat(str, ((request_struct*)requests.cur->element)->user_name);
		if(requests.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
		requests.cur = requests.cur->next;
  	}
}

void convertGroupsInviteToString(singleList requests, char str[1000]){
	str[0] = '\0';
	requests.cur = requests.root;
	while(requests.cur != NULL)
  	{
		strcat(str, ((request_struct*)requests.cur->element)->group_name);
		if(requests.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
		requests.cur = requests.cur->next;
  	}
}

void deleteRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user){

	if( strcmp( ((request_struct*)(*requests).root->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)(*requests).root->element)->user_name, user_name) == 0){
		if ( ((request_struct*)(*requests).root->element)->request_from_user == request_from_user){
			(*requests).root = deleteBegin(requests);
		}
	}
	else {
		(*requests).cur = (*requests).prev = (*requests).root;
		while((*requests).cur != NULL)
		{
			if( strcmp( ((request_struct*)(*requests).cur->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)(*requests).cur->element)->user_name, user_name) == 0){
				if ( ((request_struct*)(*requests).cur->element)->request_from_user == request_from_user){
					break;
				}
			}
			(*requests).prev = (*requests).cur;
			(*requests).cur = (*requests).cur->next;
		}
		if((*requests).cur != NULL){
			(*requests).prev->next = (*requests).cur->next;
			(*requests).cur = (*requests).prev;
		}
	}

}

singleList unJoinedMembers(singleList users, char group_name[50]){
	singleList un_joined_members;
	createSingleList(&un_joined_members);
	users.cur = users.root;
	while(users.cur != NULL)
	{
		int check = 0;
		((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.root;
		while(((user_struct*)users.cur->element)->joined_groups.cur != NULL)
		{
			if( strcmp( ((simple_group_struct*)((user_struct*)users.cur->element)->joined_groups.cur->element)->group_name, group_name) == 0)
			{
				check = 1;
				break;
			}
			((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.cur->next;
		}
		if(check == 0){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			strcpy(member_element->user_name, ((user_struct*)users.cur->element)->user_name);
			insertEnd(&un_joined_members, member_element);
		}
		users.cur = users.cur->next;
	}
	return un_joined_members;
}

singleList getRequests(singleList requests,char current_group[50]){
	singleList requests_of_group;
	createSingleList(&requests_of_group);
	requests.cur = requests.root;
	while(requests.cur != NULL){
		if(strcmp(((request_struct*)requests.cur->element)->group_name, current_group) == 0 && ((request_struct*)requests.cur->element)->request_from_user == 1){
			request_struct *request_element = (request_struct*) malloc(sizeof(request_struct));
			strcpy(request_element->group_name, ((request_struct*)requests.cur->element)->group_name);
			strcpy(request_element->user_name, ((request_struct*)requests.cur->element)->user_name);
			request_element->request_from_user = ((request_struct*)requests.cur->element)->request_from_user;
			insertEnd(&requests_of_group, request_element);
		}
		requests.cur = requests.cur->next;
	}
	return requests_of_group;
}

singleList getInvites(singleList requests,char user_name[50]){
	singleList invites_of_user;
	createSingleList(&invites_of_user);
	requests.cur = requests.root;
	while(requests.cur != NULL){
		if(strcmp(((request_struct*)requests.cur->element)->user_name, user_name) == 0 && ((request_struct*)requests.cur->element)->request_from_user == 0){
			request_struct *request_element = (request_struct*) malloc(sizeof(request_struct));
			strcpy(request_element->group_name, ((request_struct*)requests.cur->element)->group_name);
			strcpy(request_element->user_name, ((request_struct*)requests.cur->element)->user_name);
			request_element->request_from_user = ((request_struct*)requests.cur->element)->request_from_user;
			insertEnd(&invites_of_user, request_element);
		}
		requests.cur = requests.cur->next;
	}
	return invites_of_user;
}

void readRequestFile(singleList* requests){
	char username[50], group_name[50], user_name[50];
	int request_from_user ;
	FILE * f = fopen("./storage/request.txt","r");

	if (f == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	while (1)
	{	
		char c = fgetc(f);
		if (c != EOF){
			int res = fseek( f, -1, SEEK_CUR );
		}else{
			//printf("scan: stop\n");
			break;
		}
		request_struct *request = (request_struct*)malloc(sizeof(request_struct));
		fgets(group_name, 50, f);
		group_name[strlen(group_name) -1 ] = '\0';
		//printf("scan: %s\n", group_name);

		fgets(user_name, 50, f);
		user_name[strlen(user_name) -1 ] = '\0';
		//printf("scan: %s\n", owner);

		fscanf(f,"%d\n", &request_from_user);
		//printf("scan: %d\n", request_from_user);

		strcpy(request->group_name, group_name);
		strcpy(request->user_name, user_name);
		request->request_from_user = request_from_user;

		insertEnd(requests, request);
	}
	fclose(f);
}

void writeToRequestFile(singleList requests){
	// if (requests.root == NULL)
	// {
	// 	printf("No request to write.\n");
	// 	return;
	// }
	request_struct* request = NULL;
	FILE * f = fopen("./storage/request.txt","w");
	requests.cur = requests.root;
	while (requests.cur != NULL)
	{
		request = (request_struct*)(requests.cur->element);
		fprintf(f,"%s\n", request->group_name);
		fprintf(f,"%s\n", request->user_name);
		fprintf(f,"%d\n", request->request_from_user);
		requests.cur = requests.cur->next;
	}
	fclose(f);
}
int updateRequest(singleList *requests, char group_name[50], char user_name[50], int request_from_user){
	int tmp = checkRequestExit(*requests, group_name, user_name, request_from_user);
	// printf("checkRequestExit pass tmp: %d\n", tmp);
	if (tmp != 0)
		return 1 - tmp ;
	request_struct *request = (request_struct*)malloc(sizeof(request_struct));
	strcpy(request->group_name, group_name);
	strcpy(request->user_name, user_name);
	request->request_from_user = request_from_user;
	insertEnd(requests, request);
	return 1 ;
}

int checkRequestExit(singleList requests, char group_name[50], char user_name[50], int request_from_user){
	requests.cur = requests.root;
	while (requests.cur != NULL)
	{
		if(strcmp( ((request_struct*)requests.cur->element)->group_name, group_name) == 0 && strcmp( ((request_struct*)requests.cur->element)->user_name, user_name) == 0){
			if ( ((request_struct*)requests.cur->element)->request_from_user == request_from_user){
				return 1;
			}
			else {
				return 2;
			}
		}
		requests.cur = requests.cur->next; 
	}
	return 0;
}

void readFileFile(singleList *files){
	FILE *fp;
	char str_tmp[100];
	str_tmp[0] = '\0';
	fp = fopen("./storage/file.txt", "r");
	if(fp == NULL){
		fprintf(stderr, "File missing: can not find \"file.txt\".\n");
		exit(-1);
	}

	while(1){
		char c = fgetc(fp);
    	if (c != EOF){
			int res = fseek( fp, -1, SEEK_CUR );
		}else
        	break;
	
		file_struct *file = (file_struct*)malloc(sizeof(file_struct));
		// name
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->name, str_tmp);
		// owner
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->owner, str_tmp);
		// group
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->group, str_tmp);
		// uploaded
		fgets (str_tmp, 100, fp);
		str_tmp[strlen(str_tmp)-1] = '\0';
		strcpy(file->uploaded_at, str_tmp);
		fgets (str_tmp, 100, fp);
		if(str_tmp[strlen(str_tmp)-1] == '\n'){
			str_tmp[strlen(str_tmp)-1] = '\0';
		}
		file->downloaded_times = atoi(str_tmp);
		insertEnd(files, file);
	}
	fclose(fp);
}

int checkExistence(int type, singleList list, char string[50])
{
	// type = 1 check user
	// type = 2 check group
	// type = 3 check file
	switch (type)
	{
	case 1:
		// Check user
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((user_struct*)list.cur->element)->user_name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return 1;
				}
				
			}
			return 0; 
		}
		break;
	case 2:
		// Check Group
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((group_struct*)list.cur->element)->group_name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return 1;
				}
				
			}
			return 0; 
		}
		break;
	case 3:
		// Check File
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((file_struct*)list.cur->element)->name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return 1;
				}
				
			}
			return 0; 
		}
		break;

	default:
		printf("Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

void* findByName(int type, singleList list, char string[50])
{
	// type = 1 find user
	// type = 2 find group
	// type = 3 find file
	switch (type)
	{
	case 1:
		// Check user
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((user_struct*)list.cur->element)->user_name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return list.cur->element;
				}
				
			}
			return NULL; 
		}
		break;
	case 2:
		// Check Group
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((group_struct*)list.cur->element)->group_name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return list.cur->element;
				}
				
			}
			return NULL; 
		}
		break;
	case 3:
		// Check File
		{
			int i = 0;
			list.cur = list.root;
			while (list.cur != NULL)
			{
				i++;
				if(strcmp(((file_struct*)list.cur->element)->name,string) != 0)
				{
					list.cur = list.cur->next;
				}
				else {
					return list.cur->element;
				}
				
			}
			return NULL; 
		}
		break;

	default:
		printf("Type chua hop le !! (1,2 or 3)\n");
		break;
	}
}

int addMember(singleList groups, char group_name[50], char username[50]){
	printf("add %s member to %s\n", username, group_name);
	singleList members;
	createSingleList(&members);
  	groups.cur = groups.root;
	while(groups.cur != NULL)
	{
		if(strcmp(((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			simple_user_struct *member_element = (simple_user_struct*) malloc(sizeof(simple_user_struct));
			strcpy(member_element->user_name, username);
			insertEnd(&((group_struct*)groups.cur->element)->members, member_element);
			((group_struct*)groups.cur->element)->number_of_members += 1;
			return 1;
		}
		groups.cur = groups.cur->next;
	}
	return 0;
}

int addGroupToJoinedGroups(singleList users, char username[50], char group_name[50]){
	users.cur = users.root;
	while(users.cur != NULL)
	{
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			simple_group_struct *group_element = (simple_group_struct*) malloc(sizeof(simple_group_struct));
			strcpy(group_element->group_name, group_name);
			insertEnd(&((user_struct*)users.cur->element)->joined_groups, group_element);
			((user_struct*)users.cur->element)->count_group++;
			return 1;
		}
		users.cur = users.cur->next;
	}
	return 0;
}

singleList unJoinedGroups(singleList groups, singleList users, char username[50]){
	singleList joined_groups;
	createSingleList(&joined_groups);
	singleList un_joined_groups;
	createSingleList(&un_joined_groups);
	users.cur = users.root;
	while(users.cur != NULL)
	{
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			groups.cur = groups.root;
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			break;
		}
		users.cur = users.cur->next;
	}

	groups.cur = groups.root;
	while(groups.cur != NULL)
	{
		int check = 0;
		joined_groups.cur = joined_groups.root;
		while(joined_groups.cur != NULL)
		{
			if( strcmp( ((group_struct*)groups.cur->element)->group_name, ((simple_group_struct*)joined_groups.cur->element)->group_name) == 0)
			{
				check = 1;
				break;
			}
			joined_groups.cur = joined_groups.cur->next;
		}
		if(check == 0){
			simple_group_struct *group_element = (simple_group_struct*) malloc(sizeof(simple_group_struct));
			strcpy(group_element->group_name, ((group_struct*)groups.cur->element)->group_name);
			insertEnd(&un_joined_groups, group_element);
		}
		groups.cur = groups.cur->next;
	}
	return un_joined_groups;
}

void convertSimpleGroupsToString(singleList simple_group, char str[1000]){
	str[0] = '\0';
	simple_group.cur = simple_group.root;
	while(simple_group.cur != NULL)
  	{
		strcat(str, ((simple_group_struct*)simple_group.cur->element)->group_name);
		if(simple_group.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_group.cur = simple_group.cur->next;
  	}
}

void convertSimpleFilesToString(singleList simple_file, char str[1000]){
	str[0] = '\0';
	simple_file.cur = simple_file.root;
	while(simple_file.cur != NULL)
  	{
		strcat(str, ((simple_file_struct*)simple_file.cur->element)->file_name);
		if(simple_file.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_file.cur = simple_file.cur->next;
  	}
}

void convertSimpleUsersToString(singleList simple_user, char str[1000]){
	str[0] = '\0';
	simple_user.cur = simple_user.root;
	while(simple_user.cur != NULL)
  	{
		strcat(str, ((simple_user_struct*)simple_user.cur->element)->user_name);
		if(simple_user.cur->next == NULL){
			str[strlen(str)] = '\0';
		}else{
			strcat(str, "+");
		}
    	simple_user.cur = simple_user.cur->next;
  	}
}

void getBasicInfoOfGroup(singleList groups, char group_name[50], char group_info[200]){
	char temp_str[10];
	group_info[0] = '\0';
	strcat(group_info, "===========================================\n\t\t");
	strcat(group_info, group_name);
	groups.cur = groups.root;
	while(groups.cur != NULL){
		if(strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			strcat(group_info, "\n- Created by ");
			strcat(group_info, ((group_struct*)groups.cur->element)->owner);
			strcat(group_info, ".\n- ");
			sprintf(temp_str, "%d", ((group_struct*)groups.cur->element)->number_of_members);
			strcat(group_info, temp_str);
			strcat(group_info, " members.\n- ");
			sprintf(temp_str, "%d", ((group_struct*)groups.cur->element)->number_of_files);
			strcat(group_info, temp_str);
			strcat(group_info, " files shared.\n");
			strcat(group_info, "===========================================\n");
			strcat(group_info, "1. Upload\n");
			strcat(group_info, "2. Download\n");
			strcat(group_info, "3. Delete\n");
			strcat(group_info, "4. View Files\n");
			strcat(group_info, "5. Back to menu\n");
			strcat(group_info, "===========================================\n");
			group_info[strlen(group_info)] = '\0';
			break;
		}
		groups.cur = groups.cur->next;
	}
}

void createGroup(int sock, singleList * groups, user_struct *loginUser){
	char buff[100], noti[100], cmd[100];
	readWithCheck(sock, buff, 100);

	if(checkExistence(2, *groups, buff) == 1){
		strcpy(noti, "Ten nhom vua nhap da duoc su dung.");
		sendWithCheck(sock, noti, strlen(noti) + 1, 0);
	}else{
		group_struct *group_element = (group_struct*) malloc(sizeof(group_struct));
		singleList members, files;
		
		createSingleList(&members);
		createSingleList(&files);

		strcpy(group_element->group_name, buff);
		strcpy(group_element->owner, loginUser->user_name);
		printf("ownner: %s\n", group_element->owner);
		group_element->files = files;
		group_element->members = members;
		group_element->number_of_files = 0;
		group_element->number_of_members = 0;

		insertEnd(groups, group_element);

		//addMember(*groups, group_element->group_name, loginUser->user_name);
		
		addGroupToJoinedGroups(users, loginUser->user_name, group_element->group_name);

		strcpy(cmd, "mkdir ");
		strcat(cmd, "./files/\'");
		strcat(cmd, buff);
		strcat(cmd, "\'");
		system(cmd);

		strcpy(noti, "Create group successfully.\n");
		sendWithCheck(sock, noti, strlen(noti) + 1, 0);
		// writeToGroupFile(groups);
		// saveUsers(users);
	}
}

void sendCode(int sock, int code){
	char codeStr[10];
	sprintf(codeStr, "%d", code);
	printf("-->Response: %s\n", codeStr);
	sendWithCheck(sock , codeStr , strlen(codeStr) + 1 , 0 ); 
}

singleList joinedGroups(singleList users, char username[50]){
	singleList joined_groups;
	createSingleList(&joined_groups);
	users.cur = users.root;
	while(users.cur != NULL)
	{
		if(strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			break;
		}
		users.cur = users.cur->next;
	}
	return joined_groups;
}

singleList getAllFilesOfGroup(singleList groups, char group_name[50]){
	singleList files;

	createSingleList(&files);
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		if(strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			files = ((group_struct*)groups.cur->element)->files;
			break;
		}
		groups.cur = groups.cur->next;
	}
	printf("group_name: %s\n", group_name);
	return files;
}

singleList getAllMembersOfGroup(singleList groups, char group_name[50]){
	singleList members;
	createSingleList(&members);
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		if(strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			members = ((group_struct*)groups.cur->element)->members;
			break;
		}
		groups.cur = groups.cur->next;
	}
	return members;
}

singleList getFilesOwns(singleList files, char username[50]){
	singleList files_owns;
	createSingleList(&files_owns);
	files.cur = files.root;
	while (files.cur != NULL)
	{
		if(strcmp(  ((file_struct*)files.cur->element)->owner, username) == 0){
			simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
			strcpy(file_element->file_name, ((file_struct*)files.cur->element)->name);
			insertEnd(&files_owns, file_element);
		}
		files.cur = files.cur->next;
	}
	return files_owns;
}

void* SendFileToClient(int new_socket, char fname[50], char group_name[50])
{
	char path[100];
    write(new_socket, fname,256);

	path[0] = '\0';
	strcat(path, "./files/");
	strcat(path, group_name);
	strcat(path, "/");
	strcat(path, fname);
	printf("file: %s\n", path);

    FILE *fp = fopen(path,"rb");
    if(fp==NULL)
    {
        printf("File opern error");
    }   

    /* Read data from file and send it */
    while(1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[1024]={0};
        int nread = fread(buff,1,1024,fp);

        /* If read was success, send data. */
        if(nread > 0)
        {
            write(new_socket, buff, nread);
        }
		readWithCheck(new_socket, buff, BUFF_SIZE);
		if(strcasecmp(buff, "continue") != 0){
			break;
		}
        if (nread < 1024)
        {
            if (feof(fp))
            {
                printf("End of file\n");
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
}

void signUp(int sock, singleList *users);

int signIn(int sock, singleList users, user_struct **loginUser);

singleList getFilesCanDelete(singleList files, singleList groups, char group_name[], char username[]){
	singleList files_can_delete, all_files_of_group, all_files_owns;
	createSingleList(&files_can_delete); //file ma nguoi dung co the xoa
	createSingleList(&all_files_of_group); //tat ca file trong group
	createSingleList(&all_files_owns); //tat ca file nguoi dung so huu
	groups.cur = groups.root;
	while (groups.cur != NULL)
	{
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name ) == 0){
			if( strcmp( ((group_struct*)groups.cur->element)->owner, username ) == 0){
				return ((group_struct*)groups.cur->element)->files;
			}else{
				all_files_of_group = getAllFilesOfGroup(groups, group_name);
				all_files_owns = getFilesOwns(files, username);
				all_files_of_group.cur = all_files_of_group.root;
				while (all_files_of_group.cur != NULL)	
				{
					all_files_owns.cur = all_files_owns.root;
					while (all_files_owns.cur != NULL)	
					{
						if( strcmp( ((simple_file_struct*)all_files_owns.cur->element)->file_name, ((simple_file_struct*)all_files_of_group.cur->element)->file_name) == 0){
							simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
							strcpy(file_element->file_name, ((simple_file_struct*)all_files_owns.cur->element)->file_name);
							insertEnd(&files_can_delete, file_element);
						}
						all_files_owns.cur = all_files_owns.cur->next;
					}
					all_files_of_group.cur = all_files_of_group.cur->next;
				}
			}
			break;
			
		}
		groups.cur = groups.cur->next;
	}
	
	return files_can_delete;
}
// renameFile(&files, groups, current_group, file_name, new_name);
void renameFile(singleList *files, singleList groups, char group_name[], char file_name[50], char new_name[50]) {
	//rename file in singleList files
	(*files).cur = (*files).root;
	while ((*files).cur != NULL)
	{
		if( strcmp( ((file_struct*)(*files).cur->element)->name, file_name) == 0 && strcmp( ((file_struct*)(*files).cur->element)->group, group_name) == 0){
			strcpy(((file_struct*)(*files).cur->element)->name, new_name);
			break;
		}
		(*files).cur = (*files).cur->next;
	}

	groups.cur = groups.root;
	while(groups.cur != NULL){
		if( strcmp(((group_struct*)(groups).cur->element)->group_name, group_name) == 0){
			break;
		}
		groups.cur = groups.cur->next;
	}

	singleList files_of_group; 
	files_of_group = getAllFilesOfGroup(groups, group_name);
	(files_of_group).cur = (files_of_group).root;
	while ((files_of_group).cur != NULL && strcmp( ((simple_file_struct*)(files_of_group).cur->element)->file_name, file_name) != 0)
	{
		(files_of_group).cur = (files_of_group).cur->next;
	}
	strcpy(((simple_file_struct*)(files_of_group).cur->element)->file_name, new_name);
	((group_struct*)(groups).cur->element)->files = (files_of_group);

	//  rename file cmd
	printf ("cmd rename file: %s to %s\n", file_name, new_name);

	char cmd[100];
	strcpy(cmd, "mv ");
	strcat(cmd, "./files/"); strcat(cmd, group_name); strcat(cmd, "/");
	strcat(cmd, file_name);
	strcat(cmd, " ");
	strcat(cmd, "./files/"); strcat(cmd, group_name); strcat(cmd, "/");
	strcat(cmd, new_name);
	system(cmd);
	printf("cmd: %s\n", cmd);

	writeToGroupFile(groups);
	saveFiles(*files);
}


void deleteFile(singleList *files, singleList groups, char group_name[], char file_name[50]){
	//delete file in singleList files
	if( strcmp( ((file_struct*)(*files).root->element)->name, file_name) == 0 && strcmp( ((file_struct*)(*files).root->element)->group, group_name) == 0){
		(*files).root = deleteBegin(files);
	}else{
		// printf("else");
		(*files).cur = (*files).prev = (*files).root;
		while ((*files).cur != NULL)
		{
			if( strcmp( ((file_struct*)(*files).cur->element)->name, file_name) == 0 && strcmp( ((file_struct*)(*files).cur->element)->group, group_name) == 0){
				break;
			}else{
				(*files).prev = (*files).cur;
				(*files).cur = (*files).cur->next;
			}
		}
		printf("file tim thay: %s\n%s\n", ((file_struct*)(*files).cur->element)->name, ((file_struct*)(*files).cur->element)->group);
		(*files).prev->next = (*files).cur->next;
		(*files).cur = (*files).prev;
	}

	groups.cur = groups.root;
	while(groups.cur != NULL){
		if( strcmp(((group_struct*)(groups).cur->element)->group_name, group_name) == 0){
			((group_struct*)(groups).cur->element)->number_of_files -= 1;
			break;
		}
		groups.cur = groups.cur->next;
	}


	singleList files_of_group;
	files_of_group = getAllFilesOfGroup(groups, group_name);
	if( strcmp( ((simple_file_struct*)(files_of_group).root->element)->file_name, file_name) == 0){
		deleteBegin(&files_of_group);
		((group_struct*)(groups).cur->element)->files = (files_of_group);
	}else{
		(files_of_group).cur = (files_of_group).prev = (files_of_group).root;
		while ((files_of_group).cur != NULL && strcmp( ((simple_file_struct*)(files_of_group).cur->element)->file_name, file_name) != 0)
		{
			(files_of_group).prev = (files_of_group).cur;
            (files_of_group).cur = (files_of_group).cur->next;
		}
		(files_of_group).prev->next = (files_of_group).cur->next;
		(files_of_group).cur = (files_of_group).prev;
	}
	printf("delete file: %s from %s\n", file_name, group_name);
	
	// delete file in storage
	char cmd[100];
	strcpy(cmd, "rm ");
	strcat(cmd, "./files/");
	strcat(cmd, group_name);
	strcat(cmd, "/");
	strcat(cmd, file_name);
	system(cmd);
	
	writeToGroupFile(groups);
	saveFiles(*files);
}

int isFileExistInGroup(singleList groups, char group_name[], char file_name[]){
	groups.cur = groups.root;
	while(groups.cur != NULL){
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			singleList files;
			createSingleList(&files);
			files = ((group_struct*)groups.cur->element)->files;
			files.cur = files.root;
			while (files.cur != NULL)
			{
				if( strcmp( ((simple_file_struct*)files.cur->element)->file_name, file_name) == 0){
					return 1;
				}
				files.cur = files.cur->next;
			}
			break; 
		}
		groups.cur = groups.cur->next;
	}
	return 0;
}

int isOwnerOfGroup(singleList groups, char group_name[], char username[]){
	groups.cur = groups.root;
	while(groups.cur != NULL){
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name) == 0){
			if( strcmp( ((group_struct*)groups.cur->element)->owner, username) == 0){
				return 1;
			}
		}
		groups.cur = groups.cur->next;
	}
	return 0;
}

int getAllFilesOfUserInGroup(singleList *files, char group_name[50], char username[50], char all_files[20][50]){
	int number_of_files = 0;
	(*files).cur = (*files).root;
	while ((*files).cur != NULL)
	{
		if( strcmp( ((file_struct*)(*files).cur->element)->owner, username) == 0 && strcmp( ((file_struct*)(*files).cur->element)->group, group_name) == 0){
			strcpy(all_files[number_of_files], ((file_struct*)(*files).cur->element)->name);
			number_of_files++;
		}
		(*files).cur = (*files).cur->next;
	}
	return number_of_files;
}

void kickMemberOut(singleList groups, singleList users, char group_name[50], char username[50]){
	
	//delete user in singleList groups
	groups.cur = groups.root;
	while( groups.cur != NULL){
		if( strcmp( ((group_struct*)groups.cur->element)->group_name, group_name ) == 0){
			singleList members;
			createSingleList(&members);
			members = ((group_struct*)groups.cur->element)->members;
			if( strcmp(username, ((simple_user_struct*)members.root->element)->user_name) == 0){
				members.root = members.root->next;
				((group_struct*)groups.cur->element)->members = members;
			}else{
				members.cur = members.prev = members.root;
				while (members.cur != NULL && strcmp( ((simple_user_struct*)members.cur->element)->user_name, username) != 0)
				{
					members.prev = members.cur;
					members.cur = members.cur->next;
				}
				node *newNode = members.cur;
				members.prev->next = members.cur->next;
				members.cur = members.prev;
				free(newNode);
				((group_struct*)groups.cur->element)->members = members;
				
			}
			break;
		}
		groups.cur = groups.cur->next;
	}
	//delete group in joined_group
	users.cur = users.root;
	while (users.cur != NULL)
	{
		if( strcmp(((user_struct*)users.cur->element)->user_name, username) == 0){
			((user_struct*)users.cur->element)->count_group -= 1;
			singleList joined_groups;
			createSingleList(&joined_groups);
			joined_groups = ((user_struct*)users.cur->element)->joined_groups;
			if( strcmp(group_name, ((simple_group_struct*)joined_groups.root->element)->group_name) == 0){
				joined_groups.root = joined_groups.root->next;
				((user_struct*)users.cur->element)->joined_groups = joined_groups;
			}else{
				joined_groups.cur = joined_groups.prev = joined_groups.root;
				while (joined_groups.cur != NULL && strcmp(group_name, ((simple_group_struct*)joined_groups.cur->element)->group_name) != 0)
				{
					joined_groups.prev = joined_groups.cur;
					joined_groups.cur = joined_groups.cur->next;
				}
				node *newNode = joined_groups.cur;
				joined_groups.prev->next = joined_groups.cur->next;
				joined_groups.cur = joined_groups.prev;
				free(newNode);
				((user_struct*)users.cur->element)->joined_groups = joined_groups;
			}
			break;
		}
		users.cur = users.cur->next;
	}
	
	writeToGroupFile(groups);
	// saveFiles(*files);
	saveUsers(users);
}

int isUserAMember(singleList users, char group_name[50], char username[50]){
	users.cur = users.root;
	while(users.cur != NULL){
		if( strcmp( ((user_struct*)users.cur->element)->user_name, username ) == 0){
			((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.root;
			while (((user_struct*)users.cur->element)->joined_groups.cur != NULL)
			{
				if(strcmp(((simple_group_struct*)((user_struct*)users.cur->element)->joined_groups.cur->element)->group_name, group_name) == 0){
					return 1;
				}
				((user_struct*)users.cur->element)->joined_groups.cur = ((user_struct*)users.cur->element)->joined_groups.cur->next;
			}
			
		}
		users.cur = users.cur->next;
	}
	return 0;
}


int updateDownloadedTimes(singleList files, char file_name[50]){
	files.cur = files.root;
	while (files.cur != NULL)
	{
		if( strcmp( ((file_struct*)files.cur->element)->name, file_name) == 0 ){
			((file_struct*)files.cur->element)->downloaded_times += 1;
		}
		files.cur = files.cur->next;
	}
}

void signUp(int sock, singleList *users){
	char buff[BUFF_SIZE], username[50], password[50];
	int size;
	sendCode(sock, REGISTER_SUCCESS);

	while(1){
		size = readWithCheck(sock, buff, BUFF_SIZE);

		strcpy(username, buff);
		username[strlen(username) - 1] = '\0';
		if(username[strlen(username) - 2] == '\n'){
			username[strlen(username) - 2] = '\0';
		}
		printf("username: \'%s\'\n", username);
		if(checkExistence(1, *users, username) == 1){
			sendCode(sock, EXISTENCE_USERNAME);
		}else{
			sendCode(sock, REGISTER_SUCCESS);
			break;
		}
	}
	singleList groups;
	createSingleList(&groups);

	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	if(buff[strlen(buff) - 2] == '\n'){
		buff[strlen(buff) - 2] = '\0';
	}
	printf("password: %s\n", buff);
	
	strcpy(password, buff);
	user_struct *user = (user_struct*)malloc(sizeof(user_struct));
	strcpy(user->user_name, username);
	strcpy(user->password, password);
	user->count_group = 0;
	user->status = 1;
	user->joined_groups = groups;
	insertEnd(users, user);

	sendCode(sock, REGISTER_SUCCESS);
}

int signIn(int sock, singleList users, user_struct **loginUser){
	char buff[BUFF_SIZE], username[50], password[50];

	sendCode(sock, LOGIN_SUCCESS);

	while(1){
		readWithCheck(sock, buff, BUFF_SIZE);
		buff[strlen(buff) - 1] = '\0';
		printf("username: %s\n", buff);

		strcpy(username, buff);
		if(checkExistence(1, users, username) == 1){
			sendCode(sock, LOGIN_SUCCESS);
			break;
		}else{
			sendCode(sock, NON_EXISTENCE_USERNAME);
		}
	}
	readWithCheck(sock, buff, BUFF_SIZE);
	buff[strlen(buff) - 1] = '\0';
	printf("password: %s\n", buff);
	strcpy(password, buff);

	*loginUser = (user_struct*)(findByName(1, users, username));
	if(strcmp((*loginUser)->password, password) == 0){
		sendCode(sock, LOGIN_SUCCESS);
		return 1;
	}
	sendCode(sock, INCORRECT_PASSWORD);
	return 0;
}

void uploadFile(int sock, user_struct *loginUser){
	char buff[50], filePath[100], group_name[50], file_name[50], today[50];

	sendCode(sock, UPLOAD_SUCCESS);

	while(1){
		readWithCheck(sock, buff, BUFF_SIZE);
		strcpy(group_name, buff);
		printf("group_name: %s\n - %ld", buff, strlen(buff));
		readWithCheck(sock, buff, BUFF_SIZE);
		buff[strlen(buff) - 1] = '\0';
		strcpy(file_name, buff);
		if(checkExistence(3, files, file_name) == 1){
			sendCode(sock, EXISTENCE_FILE_NAME);
		}else{
			sendCode(sock, UPLOAD_SUCCESS);
			break;
		}
	}

	filePath[0] = '\0';
	strcat(filePath, "./files/");
	strcat(filePath, group_name);
	strcat(filePath, "/");
	strcat(filePath, file_name);

	receiveUploadedFile(sock, filePath);

	// get date of upload
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf( today, "%02d-%02d-%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

	file_struct *file = (file_struct*)malloc(sizeof(file_struct));
	strcpy(file->name, file_name);
	strcpy(file->group, group_name);
	strcpy(file->owner, loginUser->user_name);
	strcpy(file->uploaded_at, today);
	file->downloaded_times = 0;

	insertEnd(&files, file);
	groups.cur = groups.root;
	while(groups.cur != NULL){
		if(strcmp(group_name, ((group_struct*)groups.cur->element)->group_name) == 0){
			((group_struct*)groups.cur->element)->number_of_files += 1;
			//singleList files = ((group_struct*)groups.cur->element)->files;
			simple_file_struct *file_element = (simple_file_struct*) malloc(sizeof(simple_file_struct));
			strcpy(file_element->file_name, file_name);
			insertEnd(&((group_struct*)groups.cur->element)->files, file_element); 
		}
		groups.cur = groups.cur->next;
	} 
	writeToGroupFile(groups);
	saveFiles(files);
}

int receiveUploadedFile(int sock, char filePath[100]){
	int bytesReceived = 0;
	char recvBuff[1024], fname[100], path[100];
	FILE *fp;

	printf("Receiving file...\n");

	fp = fopen(filePath, "ab"); 
	if(NULL == fp)
	{
		printf("Error opening file\n");
		return -1;
	}
	
	double sz=1;
	/* Receive data in chunks of 256 bytes */
	while((bytesReceived = readWithCheck(sock, recvBuff, 1024)) > 0)
	{ 
		printf("\n\n\nbytes = %d\n",bytesReceived);
		sz++;
		printf("Received: %lf Mb\n",(sz/1024));
		fflush(stdout);
		fwrite(recvBuff, 1,bytesReceived,fp);

		if(bytesReceived < 1024){
			sendWithCheck(sock, "broken", 7, 0);
			break;
		}else{
			sendWithCheck(sock, "continue", 9, 0);
		}
	}
	fclose(fp);
	if(bytesReceived < 0)
	{
		printf("\n Read Error \n");
		return -1;
	}
	
	printf("\nFile OK....Completed\n");
	return 1;
}

void * handleThread(void *my_sock){
	int new_socket = *((int *)my_sock);
	int REQUEST;
	char buff[BUFF_SIZE];
	user_struct *loginUser = NULL;

	while(1){
		readWithCheck(new_socket , buff, 100);
		REQUEST = atoi(buff);
			switch (REQUEST)
			{
				case REGISTER_REQUEST:
					printf("REGISTER_REQUEST\n");
					signUp(new_socket, &users);
					saveUsers(users);
					break;
				case LOGIN_REQUEST:
					// nhan username va password
					printf("LOGIN_REQUEST\n");
					if(signIn(new_socket, users, &loginUser) == 1){
						while(REQUEST != LOGOUT_REQUEST){
							readWithCheck( new_socket , buff, BUFF_SIZE);
							REQUEST = atoi(buff);
							switch (REQUEST)
							{
							case CREATE_GROUP_REQUEST: //request code: 11
								/* code */
								printf("CREATE_GROUP_REQUEST\n");
								
								createGroup(new_socket, &groups, loginUser);
								writeToGroupFile(groups);
								saveUsers(users);
								break;

							case JOIN_GROUP_REQUEST: //request code: 12
								printf("JOIN_GROUP_REQUEST\n");
								singleList un_joined_group;
								createSingleList(&un_joined_group);
								un_joined_group = unJoinedGroups(groups, users, loginUser->user_name);
								char str[200];
								convertSimpleGroupsToString(un_joined_group, str);
								sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
								readWithCheck( new_socket , buff, 100);

								if(atoi(buff) != NO_GROUP_TO_JOIN){
									printf("nhom da chon: %s\n", buff);
									// update request to join group
									
									
									int tmp = updateRequest(&requests, buff, loginUser->user_name, 1);
									if (tmp == 1) {
										printf("update request successfully\n");
										sendCode(new_socket , REQUESTED_TO_JOIN);
										writeToRequestFile(requests);
									}
									else if (tmp == 0) {
										printf("request already exist\n");
										sendCode(new_socket , ALREADY_REQUESTED_TO_JOIN);
									}
									else if (tmp == -1) {
										printf("User has been invited to the group\n");
										sendCode(new_socket , HAS_BEEN_INVITED);
									} 									
									// if(addMember(groups, buff, loginUser->user_name) + addGroupToJoinedGroups(users, loginUser->user_name, buff) == 2){
									// 	sendCode(new_socket , JOIN_GROUP_SUCCESS);
									// 	saveUsers(users);
									// 	writeToGroupFile(groups);
									// }else{
									// 	sendWithCheck(new_socket , "something went wrong", 21, 0 );
									// }
								}else{
									printf("No group to join.\n");
								}
								break;
							case ACCESS_GROUP_REQUEST: //request code: 13
								printf("ACCESS_GROUP_REQUEST\n");
								singleList joined_group;
								createSingleList(&joined_group);
								joined_group = joinedGroups(users, loginUser->user_name);
								convertSimpleGroupsToString(joined_group, str);
								sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
								readWithCheck( new_socket , buff, 100);
								if(atoi(buff) != NO_GROUP_TO_ACCESS){
									printf("nhom da chon: %s\n", buff);
									char current_group[50];
									strcpy(current_group, buff);
									sendCode(new_socket, ACCESS_GROUP_SUCCESS);
									while(REQUEST != BACK_REQUEST){
										readWithCheck( new_socket , buff, 100);
										REQUEST = atoi(buff);
										
										switch (REQUEST)
										{
											case UPLOAD_REQUEST: //request code: 131
												if(isUserAMember(users, current_group, loginUser->user_name) == 1){
													printf("UPLOAD_REQUEST\n");
													uploadFile(new_socket, loginUser);
													writeToGroupFile(groups);
												}else{
													printf("Kicked.\n");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;

											case INVITE_MEMBER_REQUEST: 
												printf("INVITE_MEMBER_REQUEST\n");
												if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
													sendCode(new_socket, NOT_OWNER_OF_GROUP);
												}else{
													singleList unjoined_members;
													createSingleList(&unjoined_members) ;
													unjoined_members = unJoinedMembers(users, current_group);
													convertSimpleUsersToString(unjoined_members, str);
													sendWithCheck(new_socket, str, strlen(str)+1, 0);
													readWithCheck(new_socket, buff, 100);
													if(atoi(buff) != NO_MEMBER_TO_INVITE && atoi(buff)!= NO_INVITE){
														printf("group = %s, member = %s\n", current_group, buff);
														//update request to join group
														int tmp = updateRequest(&requests, current_group, buff, 0);
														if (tmp == 1) {
															printf("update request successfully\n");
															sendCode(new_socket , INVITE_SUCCESS);
															writeToRequestFile(requests);
														}
														else if (tmp == 0) {
															printf("request already exist\n");
															sendCode(new_socket , HAS_BEEN_INVITED);
														}
														else if (tmp == -1) {
															printf("User has requested to join this group\n");
															sendCode(new_socket , ALREADY_REQUESTED_TO_JOIN);
														}
													}else if(atoi(buff) == NO_MEMBER_TO_INVITE){
														printf("No member to invite.\n");
													}else if(atoi(buff) == NO_INVITE){
														printf("No invite.\n");
													}
												}	
												break;

											case APPROVE_REQUEST: 
												printf("APPROVE_REQUEST\n");
												if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
														sendCode(new_socket, NOT_OWNER_OF_GROUP);
												}
												else {
													singleList request_list;
													createSingleList(&request_list);
													request_list = getRequests(requests, current_group);
													convertUserRequestsToString(request_list, str);
													// send request list to client
													sendWithCheck(new_socket, str, strlen(str)+1, 0);

													// receive signal from client
													readWithCheck(new_socket, buff, 100);
													if(atoi(buff) != NO_REQUEST_TO_APPROVE && atoi(buff)!=NO_REQUEST_WERE_ACCEPTED){
														printf("group = %s, member = %s\n", current_group, buff);
														
														//delete request
														deleteRequest(&requests, current_group, buff, 1);
														writeToRequestFile(requests);
														// checkUser(users);
														if (addMember(groups, current_group, buff) + addGroupToJoinedGroups(users, buff, current_group) == 2) {
															sendCode(new_socket, APPROVE_SUCCESS);
															// checkUser(users);
															saveUsers(users);
															writeToGroupFile(groups);
														}
														else {
															sendWithCheck(new_socket , "something went wrong", 21, 0 );
														}
													}
												}
												break;

											case DOWNLOAD_REQUEST: //request code: 132
												if(isUserAMember(users, current_group, loginUser->user_name) == 1){
													printf("DOWNLOAD_REQUEST\n");
													singleList all_files;
													createSingleList(&all_files);
													all_files = getAllFilesOfGroup(groups, current_group);
													convertSimpleFilesToString(all_files, str);
													sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
													readWithCheck( new_socket , buff, 100);
													if(atoi(buff) != NO_FILE_TO_DOWNLOAD){
														printf("file da chon: %s\n", buff);
														SendFileToClient(new_socket, buff, current_group);
														//updateDownloadedTimes(files, buff);
													}else{
														printf("No file to download.\n");
													}
												}else{
													printf("kicked\n");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;
											case DELETE_REQUEST: //request code: 133
												// if(isUserAMember(users, current_group, loginUser->user_name) == 1){
												if(isOwnerOfGroup(groups, current_group, loginUser->user_name) == 1){
													printf("DELETE_REQUEST\n");
													singleList files_can_delete;
													createSingleList(&files_can_delete);
													files_can_delete = getFilesCanDelete(files, groups, current_group ,loginUser->user_name);
													convertSimpleFilesToString(files_can_delete, str);
													sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
													readWithCheck( new_socket , buff, 100);
													if(atoi(buff) != NO_FILE_TO_DELETE){
														printf("file da chon: %s\n", buff);
														deleteFile(&files, groups, current_group, buff);
													}else{
														printf("No file to delete\n");
													}
												}else{
													printf("kicked\n");
													sendCode(new_socket, NOT_OWNER_OF_GROUP);
													// REQUEST = BACK_REQUEST;
												}
												break;

											case RENAME_REQUEST:
												if(isOwnerOfGroup(groups, current_group, loginUser->user_name) == 1){
													printf("RENAME_REQUEST\n");
													singleList files_can_rename;
													createSingleList(&files_can_rename);
													files_can_rename = getFilesCanDelete(files, groups, current_group ,loginUser->user_name);
													convertSimpleFilesToString(files_can_rename, str);
													sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
													readWithCheck( new_socket , buff, 100);
													if (atoi(buff) != NO_FILE_TO_RENAME) {
														char file_name[50], new_name[50];
														printf("file da chon: %s\n", buff);
														char *token; 
														token = strtok(buff, ":");
														strcpy(new_name, token);
														token = strtok(NULL, ":");
														strcpy(file_name, token);

														printf("file_name = %s, new_name = %s\n", file_name, new_name);
														renameFile(&files, groups, current_group, file_name, new_name);
													}else {
														printf("No file to rename\n");
													}


												} else { 
													printf("Not owner of group\n");
													sendCode(new_socket, NOT_OWNER_OF_GROUP);
												}
												break ;

											case VIEW_FILES_REQUEST: //request code: 134
												if(isUserAMember(users, current_group, loginUser->user_name) == 1){
													printf("VIEW_FILES_REQUEST\n");
													singleList all_files;
													createSingleList(&all_files);
													all_files = getAllFilesOfGroup(groups, current_group);
													convertSimpleFilesToString(all_files, str);
													printf("%s\n", str);
													sendWithCheck(new_socket , str, strlen(str) + 1, 0 );
												}else{
													printf("kicked");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;

											case QUIT_GROUP_REQUSET: 
												printf("QUIT_GROUP_REQUSET\n");
												if (isOwnerOfGroup(groups, current_group, loginUser->user_name) == 1) {
													sendCode(new_socket, IS_OWNER_OF_GROUP);
												}
												else {
													// kick member out of group

													kickMemberOut( groups, users, current_group, loginUser->user_name);
													sendCode(new_socket, QUIT_GROUP_SUCCESS);
												}
												REQUEST = BACK_REQUEST;
												break;
											
											case VIEW_USERS_OF_GROUP_REQUEST: 
												printf("VIEW_USERS_OF_GROUP_REQUEST\n");
												if(isUserAMember(users, current_group, loginUser->user_name) == 1){
													singleList members;
													createSingleList(&members);
													members = getAllMembersOfGroup(groups, current_group);
													convertSimpleUsersToString(members, str);
													// get group owner of group
													char owner[50];
													strcpy(owner, getGroupOwner(groups, current_group));
													strcat(str, "+");
													strcat(str, owner);
													sendWithCheck(new_socket, str, strlen(str)+1, 0);
												}else{
													printf("kicked");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;

											
											case KICK_MEMBER_REQUEST:
												if(isUserAMember(users, current_group, loginUser->user_name) == 1){
													printf("KICK_MEMBER_REQUEST\n");
													if(isOwnerOfGroup(groups, current_group,loginUser->user_name) == 0){
														sendCode(new_socket, NOT_OWNER_OF_GROUP);
													}else{
														singleList members;
														createSingleList(&members);
														members = getAllMembersOfGroup(groups, current_group);
														convertSimpleUsersToString(members, str);
														sendWithCheck(new_socket, str, strlen(str)+1, 0);
														readWithCheck(new_socket, buff, 100);
														if(atoi(buff) != NO_MEMBER_TO_KICK){
															printf("group = %s kick member = %s\n", current_group, buff);
															// checkUser(users);
															kickMemberOut(groups, users,current_group, buff);
															// checkUser(users);
															singleList members1;
															createSingleList(&members1);
															members1 = getAllMembersOfGroup(groups, current_group);
															printUser(members1);
														}else{
															printf("No member to kick.\n");
														}
													}
												}else{
													printf("kicked");
													sendCode(new_socket, MEMBER_WAS_KICKED);
													REQUEST = BACK_REQUEST;
												}
												break;
											case BACK_REQUEST: //request code: 135
											/* code */
												printf("BACK_REQUEST\n");
												writeToGroupFile(groups);
												break;
											default:
												break;
										}
									}
								}
								break;

							case NOTIFICATION_REQUEST: 
								printf("NOTIFICATION_REQUEST\n");
								singleList request_list;
								createSingleList(&request_list);
								request_list = getInvites(requests, loginUser->user_name);
								convertGroupsInviteToString(request_list, str);
								sendWithCheck(new_socket, str, strlen(str)+1, 0);

								readWithCheck(new_socket, buff, 100);
								if (atoi(buff) != NO_ACCEPT_INVITE && atoi(buff) != NO_INVITE) {
									printf("group = %s\n", buff);
									//delete request
									deleteRequest(&requests, buff, loginUser->user_name, 0);
									writeToRequestFile(requests);
									if (addMember(groups, buff, loginUser->user_name) + addGroupToJoinedGroups(users, loginUser->user_name, buff) == 2) {
										sendCode(new_socket, ACCEPT_SUCCESS);
										saveUsers(users);
										writeToGroupFile(groups);
									}
									else {
										sendWithCheck(new_socket , "something went wrong", 21, 0 );
									}
								}
								break;

							case LOGOUT_REQUEST: //request code: 14
								printf("LOGOUT_REQUEST\n");
								loginUser = NULL;
								sendCode(new_socket, LOGOUT_SUCCESS);
								break;

							default:
								break;
							}
						}
					}
					break;
				default:
					break;
			}
	}
    close(new_socket);
}
