#pragma once

string searchImg(XMLElement* element, int& imageNumber) {
    if (!element) return "error";

    for (XMLNode* node = element->FirstChild(); node != nullptr; node = node->NextSibling()) {
        if (node->ToElement())
        {
            string tmpStr(node->ToElement()->Value());
            bool wereImgFound = false;

            if (tmpStr.find("img") != std::wstring::npos || tmpStr.find("image") != std::wstring::npos)
            {
                const char* src = node->ToElement()->Attribute("src");
                const char* xlink = node->ToElement()->Attribute("xlink:href");
                if (src || xlink) {
                    if (imageNumber == 0)
                    {
                        if (src)
                            return src;
                        if(xlink)
                            return xlink;
                        wereImgFound = true;
                    }
                    else
                        imageNumber--;
                }
            }
            if (!wereImgFound) {
                string tmp = searchImg(node->ToElement(), imageNumber);
                if (tmp != "error")
                    return tmp;
            }
        }
    }
    return "error";
}

bool extractFileFromZip2(const std::string& zipPath, const std::string& filePathInZip, const std::string& outputPath) 
{
    int errorp;
    zip_t* zipFile = zip_open(zipPath.c_str(), 0, &errorp);

    if (zipFile == nullptr) {
        return false;
    }

    zip_file_t* fileInZip = zip_fopen(zipFile, filePathInZip.c_str(), 0);

    if (fileInZip == nullptr) {
        zip_close(zipFile);
        return false;
    }

    std::vector<char> buffer(4096);
    std::ofstream outputFile(outputPath, std::ios::binary);

    if (!outputFile.is_open()) {
        zip_fclose(fileInZip);
        zip_close(zipFile);
        return false;
    }

    zip_int64_t bytesRead;
    while ((bytesRead = zip_fread(fileInZip, buffer.data(), buffer.size())) > 0) {
        outputFile.write(buffer.data(), bytesRead);
    }

    outputFile.close();
    zip_fclose(fileInZip);
    zip_close(zipFile);

    return true;
}

string readJPEG(std::string epub_path, int chapterNumber, int const imageNumber, jstring jCacheDir, JNIEnv* env)
{
    const char* epubFile = epub_path.c_str();
    int tmpImageNumber = imageNumber;

    int err = 0;
    zip_t* archive = zip_open(epubFile, ZIP_RDONLY, &err);
    if (!archive) {
        return "false";
    }

    string opfPath = findOpfPath(archive);
    if (opfPath.empty()) {
        zip_close(archive);
        return "false";
    }

    size_t lastSlash = opfPath.find_last_of('/');
    string basePath = (lastSlash != string::npos) ? opfPath.substr(0, lastSlash + 1) : "";
    string firstChapterPath = getChapterPath(archive, opfPath, chapterNumber);
    if (firstChapterPath.empty()) {
        zip_close(archive);
        return "false";
    }

    firstChapterPath = basePath + firstChapterPath;

    string chapterContent = readFileFromZip(archive, firstChapterPath);
    tinyxml2::XMLDocument doc;
    if (doc.Parse(chapterContent.c_str()) != tinyxml2::XML_SUCCESS) {
        return "false";
    }

    {
        tinyxml2::XMLElement* cover = doc.FirstChildElement("html")
            ->FirstChildElement("body");

        if (cover->FirstChildElement("div"))
        {
            if (cover->FirstChildElement("div")->FirstChildElement())
            {
                cover = doc.FirstChildElement("html")->FirstChildElement("body")->FirstChildElement("div");
            }
        }
        else
        {
            cover = doc.FirstChildElement("html")->FirstChildElement("body");
        }

        for (XMLElement* p = cover->FirstChildElement(); p != nullptr; p = p->NextSiblingElement())
        {
            string tmpForFullImagePath = basePath + searchImg(p, tmpImageNumber);
            std::string tempImagePath = jstringToString(env, jCacheDir) + "/temp_image.png";
            if(extractFileFromZip2(epub_path.c_str(), tmpForFullImagePath, tempImagePath))
                return tempImagePath;
        }
    }
}