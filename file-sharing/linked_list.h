#include "object.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
Các hàm khởi tạo
*/

void   createSingleList(singleList * list);
node * makeNewNode(void *e);

/*
Thêm 1 node
*/

node * insertAfter(singleList * list, void *e);
node * insertBegin(singleList * list, void *e);
node * insertEnd(singleList * list, void *e);
node * insertAtPosition(singleList * list, void *e, int n);

/*
Xóa node
*/

node * deleteBegin(singleList * list);
node * deleteEnd(singleList * list);
node * deleteAtPosition(singleList * list, int n);
node * deleteSingleList(singleList * list);


/*
Tất cả node
*/
int printUser(singleList list);
int printGroup(singleList list);

/*
*
*
Chi tiết các hàm
*
*
*/

// Tạo danh sách mới

void createSingleList(singleList * list)
{
  (*list).root = (*list).prev = (*list).cur = (*list).tail  = NULL;
}

// Tạo node mới

node * makeNewNode(void *e)
{
  node * newNode = (node *)malloc(sizeof(node));
  newNode->element = e;
  newNode->next = NULL;
  return newNode;
}

// Thêm vào cuối danh sách

node * insertEnd(singleList *list,void *e)
{
	node * newNode = makeNewNode(e);
	if((*list).root == NULL)
	{
      (*list).root = (*list).tail = newNode;
	}
	else {
    (*list).cur = (*list).prev = (*list).root;
    while((*list).cur != NULL)
      {
        (*list).prev = (*list).cur;
        (*list).cur = (*list).cur->next;
      }
    (*list).prev->next = newNode;
    (*list).tail = newNode;
  }
	return (*list).tail;
}

// Thêm vào đầu danh sách

node * insertBegin(singleList * list, void *e)
{
  node * newNode = makeNewNode(e);
  if ((*list).root == NULL)
    {
      (*list).root = (*list).tail = newNode;
    }
  else
    {
      newNode->next=(*list).root;
      (*list).root = newNode;
    }
  return (*list).root;
}

int totalSingleList(singleList list)
{
  int i=0;
  list.cur = list.root;
	while(list.cur != NULL)
      {
        i++;
        list.cur = list.cur->next;
      }
    return i;
}

// Thêm vào vị trí
node * insertAtPosition(singleList * list,void *e,int n)
{
  node *newNode = makeNewNode(e);
  if((*list).root == NULL)
    {
      (*list).root = (*list).tail = (*list).cur = newNode;
	}
  else
    {
      if(n <= 1)
        {
          insertBegin(list,newNode->element);
          return (*list).cur;
        }
      if(n > totalSingleList(*list))
        {
          insertEnd(list,newNode->element);
          return (*list).cur;
        }
      else
        {
          (*list).cur = (*list).prev = (*list).root;
          int i = 1;
          while(((*list).cur->next != NULL)&&(i <= n-1))
            {
              i++;
              (*list).prev = (*list).cur;
              (*list).cur = (*list).cur->next;
            }
          newNode->next = (*list).cur;
          (*list).prev->next = (*list).cur = newNode;
        }
    }
  return (*list).cur;
}

// Xóa node đầu tiên

node * deleteBegin(singleList * list)
{
  if((*list).root != NULL)
	{
    node * newNode = (*list).root;
		(*list).root = (*list).root->next;
		free(newNode);
	}
  return (*list).root;
}

// Xóa node cuối cùng

node * deleteEnd(singleList *list)
{
  if((*list).root != NULL)
    {
      (*list).cur = (*list).prev = (*list).root;
        while(((*list).cur->next != NULL))
          {
            (*list).prev = (*list).cur;
            (*list).cur = (*list).cur->next;
          }
      node *newNode = (*list).cur;
      (*list).cur = (*list).cur->next;
      free(newNode);
      (*list).tail = (*list).prev;
      (*list).tail->next = NULL;
      return (*list).tail;
    }
}

// Xóa tất cả
node * deleteSingleList(singleList * list)
{
  while((*list).root != NULL) deleteBegin(list);
}

// Toàn bộ danh sách

int printUser(singleList list)
{
  int i=0;
  list.cur = list.root;
	while(list.cur != NULL)
  {
    i++;
    printf("%s\n", ((user_struct*)list.cur->element)->user_name);
    list.cur = list.cur->next;
  }
  return i;
}

void printSimpleGroup(singleList group)
{
  group.cur = group.root;
	while(group.cur != NULL)
  {
    printf("%s\n", ((simple_group_struct*)group.cur->element)->group_name);
    group.cur = group.cur->next;
  }
}

int printFile(singleList list)
{
  int i=0;
  list.cur = list.root;
	while(list.cur != NULL)
      {
        i++;
        printf("%s\n", ((file_struct*)list.cur->element)->name);
        list.cur = list.cur->next;
      }
    return i;
}

int printGroup(singleList list)
{
  int i=0;
  list.cur = list.root;
	while(list.cur != NULL)
      {
        i++;
        printf("Name: %s\n", ((group_struct*)list.cur->element)->group_name);
        printf("Owner: %s\n", ((group_struct*)list.cur->element)->owner);
        printf("members = %d\n", ((group_struct*)list.cur->element)->number_of_members);
        printUser(((group_struct*)list.cur->element)->members);
        printf("files = %d\n", ((group_struct*)list.cur->element)->number_of_files);
        printFile(((group_struct*)list.cur->element)->files);
        printf("=================================================\n");
        list.cur = list.cur->next;
      }
    return i;
}

int printFiles(singleList files){
  
  file_struct* file = NULL;
  files.cur = files.root;
  while(files.cur != NULL){
    file = (file_struct*)(files.cur->element);
    printf("%s %s %s %s %d\n", file->name, file->owner, file->group, file->uploaded_at, file->downloaded_times);
    files.cur = files.cur->next;
  }
}

int printUsers(singleList users){
  singleList groups;
  user_struct* user = NULL;
  users.cur = users.root;
  while(users.cur != NULL){
    user = (user_struct*)(users.cur->element);
    printf("%s %s %d\n", user->user_name, user->password, user->status);
    printf("group: \n");
    groups = user->joined_groups;
    groups.cur = groups.root;
    while(groups.cur != NULL){
      printf("-%s\n", (char*)(groups.cur->element));
      groups.cur = groups.cur->next;
    }

    users.cur = users.cur->next;
  }
}

int saveFiles(singleList files){
  
  file_struct* file = NULL;
  files.cur = files.root;
  FILE *fp = fopen("./storage/file.txt", "w");
  if(fp == NULL){
    printf("Khong tim thay file luu tru. Luu file that bai!!\n");
  }

  while(files.cur != NULL){
    file = (file_struct*)(files.cur->element);
    fprintf(fp, "%s\n%s\n%s\n%s\n%d\n", file->name, file->owner, file->group, file->uploaded_at, file->downloaded_times);
    files.cur = files.cur->next;
  }
  fclose(fp);
  printf("luu file thanh cong.\n");

}

int saveUsers(singleList users){
  singleList groups;
  user_struct* user = NULL;

  FILE *fp = fopen("./storage/user.txt", "w");

  if(fp == NULL){
    printf("Khong tim thay file luu tru. Luu nguoi dung that bai!!\n");
  }

  users.cur = users.root;
  while(users.cur != NULL){
    user = (user_struct*)(users.cur->element);
    fprintf(fp, "%s\n%s\n%d\n", user->user_name, user->password, user->status);
    fprintf(fp,"%d\n", user->count_group);
    groups = user->joined_groups;
    groups.cur = groups.root;
    while(groups.cur != NULL){
      fprintf(fp, "%s\n", (char*)(groups.cur->element));
      groups.cur = groups.cur->next;
    }

    users.cur = users.cur->next;
  }

  fclose(fp);
  printf("luu user thanh cong.\n");
}