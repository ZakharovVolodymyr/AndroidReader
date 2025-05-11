using namespace tinyxml2;
using namespace std;

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string addNewlines(std::string& text, size_t maxLineLength) {// maxLineLength around 46
    std::string result;
    size_t count = 0;

    for (size_t i = 0; i < text.length(); ++i) {
        count++;
        if (text[i] == '\n')
            count = 0;

        if (count >= maxLineLength && text[i] == ' ')
        {
            result += '\n';
            count = 0;
        }
        else

            result += text[i];
    }

    return result;
}

bool hasLettersW(const std::wstring& str) {
    std::locale loc("");
    for (wchar_t wc : str) {
        if (std::isalpha(wc, loc)) {
            return true;
        }
    }
    return false;
}

bool hasLetters(const std::string& str) {
    std::wstring tmp = utf8_to_wstring(str);
    std::locale loc("");
    for (wchar_t wc : tmp) {
        if (std::isalpha(wc, loc)) {
            return true;
        }
    }
    return false;
}

string extractTextFromElement(XMLElement* element, string& text) {
    if (!element) return "error";
    for (XMLNode* node = element->FirstChild(); node != nullptr; node = node->NextSibling()) {
        if (node->ToText()) {
            text += node->Value();
            //text += "\n";
        }
        {
            string tmpStr(node->Value());
            if (tmpStr.find("img") != std::wstring::npos || tmpStr.find("image") != std::wstring::npos)
                return "img";
        }
        if (node->ToText());
        else if (node->ToElement())
        { 
            if (strcmp(node->Value(), "br") == 0) {
                text += " ";
            }
            else {
                if (extractTextFromElement(node->ToElement(), text) == "img")
                    return "img";
            }
        }
    }
    return "error";
}



string readFileFromZip(zip_t* epub, const string& outputDir) {

    zip_file_t* xml = zip_fopen(epub, outputDir.c_str(), 0);
    if (!xml) {
        zip_close(epub);
        return "error";
    }

    std::string buffer = "";
    char temp[4096];
    zip_int64_t bytesRead;
    while ((bytesRead = zip_fread(xml, temp, sizeof(temp))) > 0) {
        buffer.insert(buffer.end(), temp, temp + bytesRead);
    }
    buffer.push_back('\0');

    zip_fclose(xml);
    return buffer;
}

string findOpfPath(zip_t* archive) {
    string containerXML = readFileFromZip(archive, "META-INF/container.xml");
    if (containerXML.empty()) return "";

    XMLDocument doc;
    if (doc.Parse(containerXML.c_str()) != XML_SUCCESS) {
        return "";
    }

    XMLElement* rootfile = doc.FirstChildElement("container")
            ->FirstChildElement("rootfiles")
            ->FirstChildElement("rootfile");

    if (!rootfile) {
        return "";
    }

    return rootfile->Attribute("full-path");
}

string getChapterPath(zip_t* archive, const string& opfPath, int chapterNumber) {
    string contentXML = readFileFromZip(archive, opfPath);
    if (contentXML == "error" || contentXML.empty()) return "";

    XMLDocument doc;
    if (doc.Parse(contentXML.c_str()) != XML_SUCCESS) return "";

    XMLElement* manifest = doc.FirstChildElement("package")->FirstChildElement("manifest");
    XMLElement* spine = doc.FirstChildElement("package")->FirstChildElement("spine");

    if (!manifest || !spine) return "";

    // �������� ��� �������� itemref �� spine
    vector<XMLElement*> itemrefElements;
    XMLElement* itemref = spine->FirstChildElement("itemref");
    while (itemref) {
        itemrefElements.push_back(itemref);
        itemref = itemref->NextSiblingElement("itemref");
    }

    // �������� �� ��������� itemref, ������� � �������
    int foundChapters = 0;
    for (XMLElement* itemref : itemrefElements)
    {
        string itemID = itemref->Attribute("idref");

        // ���� ��������������� ���� � <manifest>
        XMLElement* item = manifest->FirstChildElement("item");
        while (item)
        {
            if (item->Attribute("id") == itemID) {
                string href = item->Attribute("href");
                {
                    if (foundChapters == chapterNumber)
                    {
                        return href;
                    }
                    foundChapters++;
                }
            }
            item = item->NextSiblingElement("item");
        }

    }

    return "<end>";
}

std::string extractEPUB(std::string epub_path, int chapterNumber) {

    const char* epubFile = epub_path.c_str();
    int err = 0;
    zip_t* archive = zip_open(epubFile, ZIP_RDONLY, &err);
    if (!archive) {
        return "error1";
    }

    string opfPath = findOpfPath(archive);
    if (opfPath.empty()) {
        zip_close(archive);
        return "error2";
    }

    size_t lastSlash = opfPath.find_last_of('/');
    string basePath = (lastSlash != string::npos) ? opfPath.substr(0, lastSlash + 1) : "";

    string firstChapterPath = getChapterPath(archive, opfPath, chapterNumber);
    if (firstChapterPath.empty()) {
        zip_close(archive);
        return "error3";
    }
    if (firstChapterPath == "<end>")
        return "<end>";

    firstChapterPath = basePath + firstChapterPath;

    string chapterContent = readFileFromZip(archive, firstChapterPath);
    if (chapterContent == "error") {
        zip_close(archive);
        return "error4";
    }

    tinyxml2::XMLDocument doc;
    if (doc.Parse(chapterContent.c_str()) != tinyxml2::XML_SUCCESS) {
        zip_close(archive);
        return "error5";
    }

    tinyxml2::XMLElement* body = doc.FirstChildElement("html")
        ->FirstChildElement("body");

    string finalText = "";

    if (body)
    {
        tinyxml2::XMLElement* cover = body;
        if (body->FirstChildElement("div"))
        {
            if (body->FirstChildElement("div")->FirstChildElement())
            {
                cover = body->FirstChildElement("div");
            }
        }

        for (XMLElement* p = cover->FirstChildElement(); p != nullptr; p = p->NextSiblingElement())
        {
            if (strcmp(p->Name(), "a") != 0 && strcmp(p->Name(), "div") != 0) {
                string extractedText = "";
                string tmp = "\t";
                extractedText += "";
                if (extractTextFromElement(p, extractedText) == "img")
                    extractedText = "<img>";

                if (!extractedText.empty() && extractedText[0] != '\n' && extractedText[0] != '\t')
                    tmp.push_back(extractedText[0]);

                for (int i = 1; i < extractedText.length(); i++)
                {
                    if (extractedText[i] != '\n' && extractedText[i] != '\t' && !(extractedText[i] == extractedText[i - 1] && extractedText[i] == ' '))
                        tmp.push_back(extractedText[i]);
                }

                tmp += "\n";
                if (hasLetters(tmp))
                    finalText += tmp;
            }
        }
    }

    zip_close(archive);

    finalText = addNewlines(finalText, 36);

    return finalText;
}