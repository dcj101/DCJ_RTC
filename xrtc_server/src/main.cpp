#include <iostream>
#include "base/conf.h"

xrtc::GeneralConf* conf = nullptr;

int init_general_conf(const char* filename) {
    if (!filename) {
        fprintf(stderr, "init_general_conf: filename is null\n");
        return -1;
    }
    conf = new xrtc::GeneralConf();
    if (!conf) {
        fprintf(stderr, "init_general_conf: conf is null\n");
        return -1;
    }
    if (load_general_conf(filename, conf) != 0) {
        fprintf(stderr, "init_general_conf: load_general_conf failed\n");
        return -1;
    }

    return 0;
}

int main() {
    int ret = init_general_conf("../conf/general.yaml");
    if (ret != 0) {
        fprintf(stderr, "main: init_general_conf failed\n");
        return -1;
    }
    return 0;
}
