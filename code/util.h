#ifndef UTIL_H_
#define UTIL_H_

struct File {
    char name[1000];
    char link[1000];
    char type[1000];
    char path[1000];
    int size;
    struct File *next;
};

struct Course {
    char id[1000];
    char name[1000];
    char info[10000];
    int info_len;
    int file_num;
    int note_num;
    struct File *file_list;
    struct File *note_list;
    struct Course *next;
};

struct User {
    char id[100];
    char password[100];
    int course_num;
    struct Course *course_list;
    struct File *upload_file;
};

int GetUserCourse();
int GetCourseInfo(struct Course*);
int GetCourseFile(struct Course*);
int GetCourseNote(struct Course*);
int DownloadFile(struct File*);
int UploadFile(struct File*);

struct File* SearchFile(const char*, struct Course*, char*, struct File*);

#endif