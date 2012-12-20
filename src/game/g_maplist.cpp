#include <vector>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>

using std::vector;
using std::string;

extern "C" {
#include "g_local.h"
}

static vector<string> map_list;

const char *G_GetRandomMap() {
    return map_list.at(rand() % map_list.size()).c_str();
}

const vector<string> *G_GetMapList() {
    return &map_list;
}

void G_CacheMapNames() {

    int numdirs = 0;
    char dirlist[8192];
	char* dirptr;
	int i;
	int dirlen;
    numdirs = trap_FS_GetFileList("maps", ".bsp", dirlist, sizeof(dirlist));

	dirptr = dirlist;
	for(i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		if(strlen(dirptr) > 4)
			dirptr[strlen(dirptr)-4] = '\0';
        if(dirptr) {
            string lowercase = dirptr;
            boost::to_lower(lowercase);
            map_list.push_back(lowercase);
        }
	}
}