//
// Created by joey on 2021/3/21.
//

#ifndef LIBDATACHANNEL_FILEREADER_H
#define LIBDATACHANNEL_FILEREADER_H

#include <fstream>
#include <iostream>
#include <vector>

class FileReader{
public:


private:
};

static void readFile(const std::string& filePath, std::function<void(const std::vector<char>&)> callback){
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

	size_t totalSize = file.tellg();
	size_t totalRead = 0;
	size_t size = 50;
	std::vector<char> buffer(size);
	if (!file.is_open()) {
		return ;
	}
	file.seekg(0, std::ios::beg);
	while (true) {
		file.read(buffer.data(), size);
		std::cout << "just read: " << file.gcount() << std::endl;
		if (file.eof()) {
            callback(std::vector<char>(buffer.begin(), buffer.begin() + file.gcount()));
			break;
		} else {
			callback(buffer);
		}
        totalRead += size;
	}
	file.close();
};

static void appendFile(const std::string& filePath, const std::vector<char>& buffer) {
	std::ofstream file(filePath, std::ios::binary | std::ios::out | std::ios::app);
	file.write(buffer.data(), buffer.size());
	file.close();
}

static std::vector<char> readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        //
    }

	return std::move(buffer);
}

static void saveFile(const std::string& destFile, const std::vector<char>& buffer) {
	std::ofstream outFile(destFile, std::ios::binary | std::ios::out | std::ios::trunc);
	outFile.write(buffer.data(), buffer.size());
	return ;
}

#endif // LIBDATACHANNEL_FILEREADER_H
