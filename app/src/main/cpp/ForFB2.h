#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <zip.h>  // ��� ���������� ZIP
#include <tinyxml2.h>  // ��� �������� XML

using namespace tinyxml2;

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

            char_array_3[0] = (unsigned char)((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
            char_array_3[1] = (unsigned char)(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
            char_array_3[2] = (unsigned char)(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

        char_array_3[0] = (unsigned char)((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
        char_array_3[1] = (unsigned char)(((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
        char_array_3[2] = (unsigned char)(((char_array_4[2] & 0x3) << 6) + char_array_4[3]);

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

void saveImageFromBinaryId(tinyxml2::XMLDocument& doc, const std::wstring& id, jstring jCacheDir, JNIEnv* env)
{
    static int imageCounter = 1;
    std::string id_utf8(id.begin(), id.end());
    const tinyxml2::XMLElement* binaryElement = doc.FirstChildElement("FictionBook")->FirstChildElement("binary");
    while (binaryElement)
    {
        const char* attrId = binaryElement->Attribute("id");
        if (attrId && id_utf8 == attrId) {
            const char* base64Text = binaryElement->GetText();
            if (base64Text) {
                std::string base64data(base64Text);
                std::vector<unsigned char> decodedData = base64_decode(base64data);
                std::string filename = jstringToString(env, jCacheDir) + "/" + std::to_string(imageCounter) + ".png";
                std::ofstream outFile(filename, std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(decodedData.data()), decodedData.size());
                outFile.close();
                imageCounter++;
            }
            return;
        }
        binaryElement = binaryElement->NextSiblingElement("binary");
    }
}

void splitIntoChaptersFB2(const std::vector<std::string>& words, std::vector<std::string>& chapters)
{
    std::string currentChapter;
    int sectionDepth = 0;

    for (const std::string& word : words) {
        bool isItTag = false;
        if (!currentChapter.empty()) currentChapter += " ";

        if (word.find("<section>") != std::wstring::npos)
        {
            chapters.push_back(currentChapter);
            currentChapter.clear();
            isItTag = true;
        }
        if (word.find("</section>") != std::wstring::npos)
        {
            isItTag = true;
        }
        if (word.find("<p>") != std::wstring::npos)
        {
            currentChapter += "\t";
            isItTag = true;
        }
        if (word.find("</p>") != std::wstring::npos)
        {
            currentChapter += "\n";
            isItTag = true;
        }
        if(!isItTag)
            currentChapter += word;
    }
}
void extractTextFB2(const tinyxml2::XMLElement* element, std::vector<std::string>& words, tinyxml2::XMLDocument& doc, jstring jCacheDir, JNIEnv* env)
{
    if (!element) return;
    std::string tmp = "<" + std::string(element->Name()) + ">";

    if(tmp.find("<section>") != std::wstring::npos || tmp.find("<p>") != std::wstring::npos)
    words.push_back("<" + std::string(element->Name()) + ">");

    for (const tinyxml2::XMLNode* node = element->FirstChild(); node != nullptr; node = node->NextSibling()) {
        if (node->ToText()) {
            const char* text = node->Value();
            if (text) {
                std::istringstream stream(text);
                std::string word;
                while (stream >> word) {
                    words.push_back(word);
                }
            }
        }
        else if (const tinyxml2::XMLElement* childElement = node->ToElement()) {
            extractTextFB2(childElement, words, doc, jCacheDir, env);
        }
    }

    if (tmp.find("<section>") != std::wstring::npos || tmp.find("<p>") != std::wstring::npos)
    words.push_back("</" + std::string(element->Name()) + ">");

    if (tmp.find("<image>") != std::wstring::npos)
    {
        const char* href = element->Attribute("l:href");
        if (href) {
            std::string hrefStr(href);

            // hrefStr �������� ��� "#img1", ���� ������ #
            if (!hrefStr.empty() && hrefStr[0] == '#') {
                {
                    hrefStr = hrefStr.substr(1); // ������� #

                    // ��������� string -> wstring
                    std::wstring id(hrefStr.begin(), hrefStr.end());

                    saveImageFromBinaryId(doc, id, jCacheDir, env);
                }
                words.push_back("\n<img>\n");
            }
        }
    }
}

std::string extractFB2(std::string fb2_path, int chapterNumber, jstring jCacheDir, JNIEnv* env)
{
    std::ifstream fopen(fb2_path);
    if (!fopen.is_open()) {
        std::cerr << "Error: failed to open " << fb2_path << std::endl;
        return "error";
    }
    std::stringstream buffer;
    std::string line;
    while (std::getline(fopen, line)) {
        buffer << line << "\n";
    }
    std::string xmlContent = buffer.str();
    fopen.close();
    if (xmlContent.empty()) {
        std::cerr << "Error: failed to read FB2 content" << std::endl;
        return "error";
    }
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error: failed to parse FB2 content: " << doc.ErrorStr() << std::endl;
        return "error";
    }
    std::vector<std::string> words;
    const tinyxml2::XMLElement* root = doc.RootElement();
    extractTextFB2(root, words, doc, jCacheDir, env);
    std::vector<std::string> chapters;
    splitIntoChaptersFB2(words, chapters);
    std::string finalText = addNewlines(chapters[chapterNumber], 120);
    return finalText;
}