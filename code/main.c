#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "util.h"

extern struct User *user;
extern char buffer_path[];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int thfs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path;
    if (strcmp(curpath, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2 + user->course_num + 1;
        return 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + 1;
    if (strstr(curpath, "Upload") == curpath) {
        if (strlen("Upload") == strlen(curpath)) {
            stbuf->st_mode = S_IFDIR | 0777;
            stbuf->st_nlink = 2;
            return 0;
        }
        else {
            if (user->upload_file != NULL) {
                if (strcmp(curpath + 7, user->upload_file->name) == 0) {
                    stbuf->st_mode = S_IFREG | 0777;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = user->upload_file->size;
                    return 0;
                }
            }
            else {
                return -ENOENT;
            }
        }
    }
    struct Course *course = user->course_list;
    while (course) {
        if (strstr(curpath, course->name) == curpath) {
            if (strlen(course->name) == strlen(curpath)) {
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 2 + 4;
                return 0;
            }
            else {
                break;
            }
        }
        course = course->next;
    }
    if (!course) return -ENOENT;
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + strlen(course->name) + 1;
    if (strstr(curpath, "Note") == curpath) {
        if (strlen("Note") == strlen(curpath)) {
            stbuf->st_mode = S_IFDIR | 0777;  
            stbuf->st_nlink = 2 + course->note_num;  
            return 0;
        }
        else {
            struct File *file = NULL;
            if ((file = SearchFile(path, course, "Note", course->note_list)) != NULL) {
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = file->size;
                return 0;
            } 
            else {
                return -ENOENT;
            }
        }
    }
    else if (strstr(curpath, "File") == curpath) {
        if (strlen("File") == strlen(curpath)) {
            stbuf->st_mode = S_IFDIR | 0777;    
            stbuf->st_nlink = 2 + course->file_num;    
            return 0;
        }
        else {
            struct File *file = NULL;
            if((file = SearchFile(path, course, "File", course->file_list)) != NULL) {
                stbuf->st_mode = S_IFREG | 0777;    
                stbuf->st_nlink = 1;    
                stbuf->st_size = file->size; 
                return 0;
            }
            else {
                return -ENOENT;
            }
        }
    }
    else if (strstr(curpath, "Info") == curpath) {
        if (strlen("Info") == strlen(curpath)) {
            stbuf->st_mode = S_IFREG | 0777;    
            stbuf->st_nlink = 1;    
            stbuf->st_size = course->info_len; 
            return 0;
        } 
        else {
            return -ENOENT;
        }
    }
    return -ENOENT;
}

static int thfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path;
    if (strcmp(curpath, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        struct Course *course = user->course_list;
        while(course) {
            filler(buf, course->name, NULL, 0);
            course = course->next;
        }
        filler(buf, "Upload", NULL, 0);
        return 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + 1;
    if (strstr(curpath, "Upload") == curpath) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        if (user->upload_file != NULL) {
            filler(buf, user->upload_file->name, NULL, 0);
        }
        return 0;
    }
    struct Course *course = user->course_list;
    while (course) {
        if (strstr(curpath, course->name) == curpath) {
            if (strlen(course->name) == strlen(curpath)) {
                filler(buf, ".", NULL, 0);
                filler(buf, "..", NULL, 0);
                filler(buf, "Note", NULL, 0);
                filler(buf, "Info", NULL, 0);
                filler(buf, "File", NULL, 0);
                return 0;
            }
            else {
                break;
            }
        }
        course = course->next;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + strlen(course->name) + 1;
    if (strstr(curpath, "File") == curpath) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        GetCourseFile(course);
        struct File *file = course->file_list;
        while (file) {
            filler(buf, file->name, NULL, 0);
            file = file->next;
        }
        return 0;
    }
    else if(strstr(curpath, "Note") == curpath) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        GetCourseNote(course);
        struct File *file = course->note_list;
        while (file) {
            filler(buf, file->name, NULL, 0);
            file = file->next;
        }
        return 0;
    }
    return 0;
}

static int thfs_open(const char *path, struct fuse_file_info *fi) {
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path + 1;
    if (strstr(curpath, "Upload") == curpath) {
        if (user->upload_file != NULL) {
            if (strcmp(user->upload_file->name, curpath + 7) == 0) {
                pthread_mutex_lock(&mutex);
                UploadFile(user->upload_file);
                pthread_mutex_unlock(&mutex);
                return 0;
            }
        }
    }
    struct Course *course = user->course_list;
    while(course) {
        if(strstr(curpath, course->name) == curpath) break;
        course = course->next;
    }
    if (!course) return -ENOENT;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + strlen(course->name) + 1;
    if (strstr(curpath, "Info") == curpath) {
        if (strlen("Info") == strlen(curpath)) {
            return 0;
        }
    }
    else if (strstr(curpath, "File") == curpath) {
        struct File *file = NULL;
        if ((file = SearchFile(path, course, "File", course->file_list)) != NULL) {
            return 0;
        }
    }
    else if (strstr(curpath, "Note") == curpath) {
        struct File *file = NULL;
        if ((file = SearchFile(path, course, "Note", course->note_list)) != NULL) {
            return 0;
        }
    }
    return -ENOENT;
}

static int thfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    (void) fi;
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path + 1;
    if (strstr(curpath, "Upload") == curpath) {
        if (user->upload_file != NULL) {
            if (strcmp(user->upload_file->name, curpath + 7) == 0) {
                FILE *fp = fopen(user->upload_file->path, "rb");
                if (!fp) return 0;
                size = fread(buf, sizeof(char), size, fp);
                fclose(fp);
                return size;
            }
        }
    }
    struct Course *course = user->course_list;
    while (course) {
        if (strstr(curpath, course->name) == curpath) break;
        course = course->next;
    }
    if (!course) return 0;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = curpath + strlen(course->name) + 1;
    if (strstr(curpath, "Info") == curpath) {
        if (strlen("Info") == strlen(curpath)) {
            len = course->info_len;
            if (offset > len) return 0;
            if (offset + size > len) size = len - offset;
            memcpy(buf, course->info + offset, size);
            return size;
        }
    }
    else if (strstr(curpath, "File") == curpath) {
        struct File *file = NULL;
        if ((file = SearchFile(path, course, "File", course->file_list)) != NULL) {
            if (strlen(file->path) == 0) { 
                pthread_mutex_lock(&mutex);
                strcpy(file->path, buffer_path);
                strcat(file->path, file->name);
                DownloadFile(file);
                pthread_mutex_unlock(&mutex);
            }
            if (offset > file->size) return 0;
            FILE *fp = fopen(file->path, "rb");
            if (!fp) return 0;
            fseek(fp, offset, SEEK_SET);
            if (offset + size > file->size) size = file->size - offset;
            size = fread(buf, sizeof(char), size, fp);
            fclose(fp);
            return size;
        }
    }
    else if (strstr(curpath, "Note") == curpath) {
        struct File *file = NULL;
        if ((file = SearchFile(path, course, "Note", course->note_list)) != NULL) {
            if (offset > file->size) return 0;
            FILE *fp = fopen(file->path, "rb");
            if(!fp) return 0;
            fseek(fp, offset, SEEK_SET);
            if(offset + size > file->size) size = file->size - offset;
            size = fread(buf, sizeof(char), size, fp);
            fclose(fp);
            return size;
        }
    }
    return 0;
}

static int thfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    (void) fi;
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path;
    if (strcmp(curpath, "/") == 0) return 0;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = path + 1;
    if (strstr(curpath, "Upload") == curpath) {
        struct File* file = user->upload_file;
        char tmp_buffer_path[1000];
        strcpy(tmp_buffer_path, buffer_path);
        strcat(tmp_buffer_path, "_upload_");
        strcat(tmp_buffer_path, file->name);
        strcpy(file->path, tmp_buffer_path);
        FILE *fp = fopen(file->path, "w+");
        if (!fp) return 0;
        fseek(fp, offset, SEEK_SET);
        size = fwrite(buf, sizeof(char), size, fp);
        file->size += size;
        fclose(fp);
        return size;
        return 0;
    }
    struct Course *course = user->course_list;
    while (course) {
        if (strstr(curpath, course->name) == curpath) {
            if (strlen(course->name) == strlen(curpath)) return 0;
            else break;
        }
        course = course->next;
    }
    if (!course) return 0;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = path + strlen(course->name) + 1;
    if (strstr(curpath, "Note") == curpath) return 0;
    else if (strstr(curpath, "Info") == curpath) return 0;
    else if(strstr(curpath, "File") == curpath) return 0;
    return 0;
}

static int thfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    int fd;
    (void) fi;
    //////////////////////////////////////////////////////////////////////////////////////////////
    const char *curpath = path;
    if (strcmp(curpath, "/") == 0) return 0;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = path + 1;
    if (strstr(curpath, "Upload") == curpath) {
        if (user->upload_file) free(user->upload_file);
        struct File *file = (struct File*)malloc(sizeof(struct File));
        memset(file, 0, sizeof(struct File));
        const char *filename = curpath + 7;
        strcpy(file->name, filename);
        file->size = 0;
        user->upload_file = file;
        return 0;
    }
    struct Course *course = user->course_list;
    while (course) {
        if (strstr(curpath, course->name) == curpath) {
            if (strlen(course->name) == strlen(curpath)) return 0;
            else break;
        }
        course = course->next;
    }
    if (!course) return 0;
    //////////////////////////////////////////////////////////////////////////////////////////////
    curpath = path + strlen(course->name) + 1;
    if (strstr(curpath, "Note") == curpath) return 0;
    else if (strstr(curpath, "Info") == curpath) return 0;
    else if(strstr(curpath, "File") == curpath) return 0;
    return 0;
}

static struct fuse_operations thfs_oper = {
    .getattr    = thfs_getattr,
    .readdir    = thfs_readdir,
    .open       = thfs_open,
    .read       = thfs_read,
    .write      = thfs_write,
    .create     = thfs_create,
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("<Usage> ./thfs -d mountpoint username password\n");
        return -1;
    }
    mkdir(buffer_path, 0777);
    user = (struct User*)malloc(sizeof(struct User));
    strcpy(user->id, argv[3]);
    strcpy(user->password, argv[4]);
    user->course_list = NULL;
    user->upload_file = NULL;
    GetUserCourse();
    return fuse_main(3, argv, &thfs_oper, NULL);
}
