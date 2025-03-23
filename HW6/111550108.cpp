/*
Student No.: 111550108
Student Name: Chia-Yu Wu
Email: amberwu0411@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not 
supposed to be posted to a public server, such as a 
public GitHub repository or a public web page.
*/
#define FUSE_USE_VERSION 30
#include<iostream>
#include<fuse.h>
#include<string.h>
#include<sys/types.h>
#include<fstream>
#include<cstdint>
#include<regex>
#include<set>
#include<unistd.h>
using namespace std;

class Tar_Header{
public:
    char name[100];             /*   0 */
    char mode[8];               /* 100 */
    char uid[8];                /* 108 */
    char gid[8];                /* 116 */
    char size[12];              /* 124 */
    char mtime[12];             /* 136 */
    char chksum[8];             /* 148 */
    char typeflag;              /* 156 */
    char linkname[100];         /* 157 */
    char magic[6];              /* 257 */
    char version[2];            /* 263 */
    char uname[32];             /* 265 */
    char gname[32];             /* 297 */
    char devmajor[8];           /* 329 */
    char devminor[8];           /* 337 */
    char prefix[155];           /* 345 */
                                /* 500 */
};

map<string, set<string>> file_dir;      // file_dir[dir] = {file, file, file...};
map<string, struct stat *> file_attr;   // file_attr[filename] = file attribute
map<string, char *> file_cont;          // file_cont[filename] = file content
map<string, char *> file_link;          // file_cont[filename] = file link

int oct_to_dec(char *oct, int sz){
    int x = 0;
    for(int i=0;i<sz && oct[i]>='0' && oct[i]<='9';i++){
        x *= 8;
        x += (int)(oct[i] - '0');
    }
    return x;
}

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    string str_path(path);
    if(str_path[str_path.size() - 1] != '/'){str_path += '/';}

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);
    if(file_dir.count(str_path) == 1){
        for(string f : file_dir[str_path]){
            if(f[f.size()-1] == '/'){
                f[f.size()-1] = '\0';
            }
            filler(buffer, f.c_str(), NULL, 0);
        }
    }
    return 0;
}

int my_getattr(const char *path, struct stat *st) {
    memset(st, 0, sizeof(struct stat));
    string str_path(path);

    // root dir
    if(str_path == "/"){
        st->st_mode = S_IFDIR | 0444;
        return 0;
    }

    // soft link
    if(file_link.count(str_path) == 1){
        memcpy(st, file_attr[str_path], sizeof(struct stat));
        st->st_mode = S_IFLNK | st->st_mode;
        // st->st_size = strlen(file_link[str_path]);
        return 0;
    }

    // reg file
    if(file_attr.count(str_path) == 1){
        memcpy(st, file_attr[str_path], sizeof(struct stat));
        st->st_mode = S_IFREG | st->st_mode;
        return 0;
    }

    // other dir
    str_path += '/';
    if(file_attr.count(str_path) == 1){
        memcpy(st, file_attr[str_path], sizeof(struct stat));
        st->st_mode = S_IFDIR | st->st_mode;
        return 0;
    }
    return -ENOENT;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    string str_path(path);
    if(file_link.count(str_path) == 1){
        str_path = file_link[str_path];
    }
    
    if(file_cont.count(str_path) == 1){
        size_t sz = file_attr[str_path]->st_size;
        if(offset >= sz){return 0;}
        if(offset + size > sz){
            memcpy(buffer, file_cont[str_path] + offset, sz - offset);
            return sz - offset;
        }
        memcpy(buffer, file_cont[str_path] + offset, size);
        return size;
    }
    return 0;
}

int my_readlink(const char *path, char *buffer, size_t size) {
    // input path = /softlink_b.txt
    // output (buffer) target = ./softlink/b.txt
    // header.linkname  filename        file_link[filename]
    // ./softlink/b.txt /softlink_b.txt ./softlink/b.txt
    string str_path(path);
    if(file_link.count(str_path) == 1){
        string s = file_link[str_path];
        // string s = "./softlink/b.txt";
        // memcpy(buffer, file_link[str_path], size);
        memcpy(buffer, s.c_str(), size);
        return 0;
    }
    return -ENOENT;
}

static struct fuse_operations op = {};

int main(int argc, char *argv[]){
    // open file
    ifstream tar("test.tar", ios::binary);
    if(!tar){
        cout<<"failed to open tar file: test.tar"<<endl;
        exit(EXIT_FAILURE);
    }

    char tar_eof[512];
    memset(tar_eof, '\0', 512);

    // read file and construct fs
    while(!tar.eof()){
        Tar_Header header{};
        tar.read((char*)&header, 512);

        // tar end
        if(memcmp(&header, tar_eof, 512) == 0){break;}

        // get header
        int header_sz = oct_to_dec(header.size, 12);
        char *buffer = new char[header_sz + 1];
        tar.read(buffer, header_sz);
        buffer[header_sz] = '\0';

        // handel file
        string filename(header.name);
        filename.insert(0, "/");
        smatch match;
        if(filename[filename.size() - 1] == '/'){
            regex_search(filename, match, regex("(.*/)(.*/)$"));
        }
        else{
            regex_search(filename, match, regex("(.*/)([^/]*)$"));
        }
        file_dir[match.str(1)].insert(match.str(2));
        struct stat *st = new struct stat;
        st->st_mode = oct_to_dec(header.mode, 8);
        st->st_uid = oct_to_dec(header.uid, 8);
        st->st_gid = oct_to_dec(header.gid, 8);
        st->st_size = oct_to_dec(header.size, 12);
        st->st_mtime = oct_to_dec(header.mtime, 12);
        st->st_nlink = 0;
        st->st_blocks = 0;

        if(file_attr.count(filename) == 1){
            delete[] file_attr[filename];
            file_attr.erase(filename);
        }
        if(file_cont.count(filename) == 1){
            delete[] file_cont[filename];
            file_cont.erase(filename);
        }
        file_attr[filename] = st;
        file_cont[filename] = buffer;
        if(header.typeflag == '2'){
            // cout<<header.linkname<<' '<<filename<<' ';

            file_link[filename] = new char[105];
            strcpy(file_link[filename], header.linkname);
            // file_link[filename] = header.linkname;
            // cout<<file_link[filename]<<'\n';
            // string s = "fjwep";
            // file_link[filename] = (char*)s.c_str();
            // header.linkname  filename        file_link[filename]
            // ./softlink/b.txt /softlink_b.txt ./softlink/b.txt
        }
        tar.ignore((512 - (header_sz % 512)) % 512);
    }

    // for(const auto& pair : file_link){
    //     cout<<pair.first<<", "<<pair.second<<endl;
    // }

    // return 0;

    memset(&op, 0, sizeof(op));
    op.getattr = my_getattr;
    op.readdir = my_readdir;
    op.read = my_read;
    op.readlink = my_readlink;
    return fuse_main(argc, argv, &op, NULL);
}

/*
[Correct]: 1 3 4
[ Wrong ]: 2 5 6
2.txt: total != 0
*/

/*
result
==== 1.txt ====
AC
==== 2.txt ====
跑出奇怪的字元
==== 3.txt ====
cat: tarfs/dir1/dir2/hello_world.html: No such file or directory
只能過第三行
==== 4.txt ====
WA, No such file or directory
==== 5.txt ====
WA
==== 6.txt ====
WA, No such file or directory
*/

// permission denied
// chmod -R 755 ./testcase