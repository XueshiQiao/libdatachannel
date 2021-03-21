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

static std::vector<char> readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        //
    }

	return std::move(buffer);
//	char* data = new char[size];
//	if (file.read(data, size)) {
//		//
//	}
//	std::cout << data << std::endl;
	//finish
}

static void saveFile(const std::string& destFile, const std::vector<char>& buffer) {
	std::ofstream outFile(destFile, std::ios::binary | std::ios::out | std::ios::trunc);
	outFile.write(buffer.data(), buffer.size());
	return ;
}

#endif // LIBDATACHANNEL_FILEREADER_H
