#include "conf.h"
#include "yaml-cpp/yaml.h"
#include <iostream>
namespace xrtc {
int load_general_conf(const char* filename, GeneralConf* conf) {
    if (!filename || !conf) {
        fprintf(stderr, "load_general_conf: filename or conf is null\n");
        return -1;
    }
    
    conf->log_dir = "./log";
    conf->log_name = "undefined";
    conf->log_level = "info";
    conf->log_to_stderr = false;
    conf->ice_min_port = 0;
    conf->ice_max_port = 65535;

    YAML::Node config = YAML::LoadFile(filename);
    std::cout << "config: " << config << std::endl;
    try {
        conf->log_dir = config["log"]["log_dir"].as<std::string>();
        conf->log_name = config["log"]["log_name"].as<std::string>();
        conf->log_level = config["log"]["log_level"].as<std::string>();
        conf->log_to_stderr = config["log"]["log_to_stderr"].as<bool>();
        conf->ice_min_port = config["ice"]["min_port"].as<int>();
        conf->ice_max_port = config["ice"]["max_port"].as<int>();
    } catch (const YAML::Exception& e) {
        fprintf(stderr, "load_general_conf: YAML exception - %s\n", e.what());
        return -1;
    }

    return 0;
}
}