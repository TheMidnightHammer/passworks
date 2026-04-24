/*
 * Copyright (c) 2026 MidnightHammer-code
 * This source code is licensed under the GPL 3.0 license
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct PasswordEntry {
    std::string username;
    std::string website;
    std::string password;
};

std::string transform(std::string data, const std::string& key) {
    std::string output = data;
    for (size_t i = 0; i < data.size(); ++i) {
        output[i] = data[i] ^ key[i % key.size()];
    }
    return output;
}

std::string simpleHash(std::string p) {
    for(char &c : p) c = (c ^ 0x55) + 2; 
    return p;
}

std::string getOrSetMaster() {
    std::string masterFile = "config.bin";
    std::string storedHash;
    std::string inputPass;

    std::ifstream checkFile(masterFile, std::ios::binary);
    
    if (!checkFile) {
        std::cout << "--- FIRST RUN DETECTED ---\n";
        std::cout << "Set your Master Password: ";
        std::getline(std::cin, inputPass);
        
        std::ofstream outFile(masterFile, std::ios::binary);
        std::string hashed = simpleHash(inputPass);
        outFile << hashed;
        outFile.close();
        
        std::cout << "Master Password set successfully!\n\n";
        return inputPass;
    } else {
        std::getline(checkFile, storedHash);
        checkFile.close();

        while (true) {
            std::cout << "Enter Master Password: ";
            std::getline(std::cin, inputPass);
            if (simpleHash(inputPass) == storedHash) {
                return inputPass;
            }
            std::cout << "Incorrect! Try again.\n";
        }
    }
}

void addPassword(const std::string& filename, const std::string& key) {
    PasswordEntry entry;
    std::cout << "Enter Website: "; std::getline(std::cin, entry.website);
    std::cout << "Enter Username: "; std::getline(std::cin, entry.username);
    std::cout << "Enter Password: "; std::getline(std::cin, entry.password);

    std::string fullLine = entry.website + " | " + entry.username + " | " + entry.password;
    std::string encrypted = transform(fullLine, key);

    std::ofstream file(filename, std::ios::app | std::ios::binary);
    if (file.is_open()) {
        size_t size = encrypted.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(encrypted.c_str(), size);
        file.close();
        std::cout << "Saved successfully!\n";
    }
}

void viewAndRemove(const std::string& filename, const std::string& key, bool deleteMode) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "No records found.\n";
        return;
    }

    std::vector<std::string> allEncryptedRecords;
    size_t size;
    while (file.read(reinterpret_cast<char*>(&size), sizeof(size))) {
        std::string buffer(size, ' ');
        file.read(&buffer[0], size);
        allEncryptedRecords.push_back(buffer);
    }
    file.close();

    std::cout << "\n--- Decrypted Records ---\n";
    for (size_t i = 0; i < allEncryptedRecords.size(); ++i) {
        std::cout << i + 1 << ". " << transform(allEncryptedRecords[i], key) << "\n";
    }

    if (deleteMode && !allEncryptedRecords.empty()) {
        int index;
        std::cout << "Enter the number to remove: ";
        std::cin >> index;
        if (index > 0 && index <= (int)allEncryptedRecords.size()) {
            std::ofstream outFile(filename, std::ios::binary);
            for (int i = 0; i < (int)allEncryptedRecords.size(); ++i) {
                if (i != (index - 1)) {
                    size_t s = allEncryptedRecords[i].size();
                    outFile.write(reinterpret_cast<const char*>(&s), sizeof(s));
                    outFile.write(allEncryptedRecords[i].c_str(), s);
                }
            }
            std::cout << "Record removed.\n";
        }
    }
}

int main() {
    std::string filename = "passwords.dat";
    std::cout << "Welcome to Passworks\nBy TheMidnightHammer\n\n";

    std::string masterKey = getOrSetMaster();

    int choice;
    while (true) {
        std::cout << "\n1. Add\n2. View\n3. Remove\n4. Exit\nChoice: ";
        if (!(std::cin >> choice)) break;
        std::cin.ignore();

        if (choice == 4) break;

        switch (choice) {
            case 1: addPassword(filename, masterKey); break;
            case 2: viewAndRemove(filename, masterKey, false); break;
            case 3: viewAndRemove(filename, masterKey, true); break;
        }
    }
    return 0;
}