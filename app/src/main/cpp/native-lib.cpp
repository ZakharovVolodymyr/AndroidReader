#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <vector>
#include <fstream>
#include "zip.h"
#include <tinyxml2.h>
#include <sstream>
#include <locale>
#include <codecvt>
#include <unistd.h>
#include <iostream>
#include <string>
#include <regex>

#include "ForJava.h"
#include "ForEPUB.h"
#include "ForFB2.h"
#include "ForJPG-EPUB.h"
#include "ForPages.h"

//#include "src/mobi.h"

extern "C"
{
    JNIEXPORT jstring JNICALL Java_com_example_myapplication_MainActivity_openBook(JNIEnv* env,jobject thiz,
       jstring jpath, jobject jChapterNumber, jobject jCurrentPage,
       jobject jCurrentImage, jobject jPreviousChapterNumber, jstring jCacheDir)
    {

        const char* fileEPUB = env->GetStringUTFChars(jpath, nullptr);

        int chapterNumber = integerHolderToInt(env, jChapterNumber);
        std::wstring wholeChapter;
        int amountOfPagesInChapter;
        int currentPage = integerHolderToInt(env, jCurrentPage);//TODO add to Java code
        double currentImage = doubleHolderToDouble(env, jCurrentImage);//TODO add to Java code
        int amountOfImages = 0;
        int previousChapterNumber = integerHolderToInt(env,
                                                       jPreviousChapterNumber);//TODO add to Java code

        string tmpStrForCheck = fileEPUB;
        if (tmpStrForCheck.find("epub") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
        if (tmpStrForCheck.find("fb2") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractFB2(fileEPUB, chapterNumber, jCacheDir, env));

        int numberOfLines = 26;
        wstring text;
        std::vector<std::wstring> pages = returnPages(wholeChapter, numberOfLines, amountOfImages);
        text = pages[0];
        amountOfPagesInChapter = pages.size();
        bool showText = true;
        string pathToImage = openingBook(chapterNumber, wholeChapter, numberOfLines, pages,
                                         currentPage, amountOfPagesInChapter, text, amountOfImages,
                                         fileEPUB, previousChapterNumber, currentImage, showText,
                                         1000, 1000, jCacheDir, env);

        {//this block is used to change Java values
            changeJavaIntLinkTo(env, jChapterNumber, chapterNumber);
            changeJavaIntLinkTo(env, jCurrentPage, currentPage);
            changeJavaDoubleLinkTo(env, jCurrentImage, currentImage);
            changeJavaIntLinkTo(env, jPreviousChapterNumber, previousChapterNumber);
        }

        if (showText) {
            using convert_type = std::codecvt_utf8<wchar_t>;
            wstring_convert<convert_type, wchar_t> conventer;
            string tmp = conventer.to_bytes(text);
            return stdStringToJstring(env, tmp);
        } else {
            return stdStringToJstring(env, pathToImage);
        }
    }

JNIEXPORT jstring JNICALL Java_com_example_myapplication_MainActivity_changePage(JNIEnv* env,jobject thiz,
   jobject javaAssetManager, jstring jpath, jobject jChapterNumber,
   jobject jCurrentPage, jobject jCurrentImage,
   jobject jPreviousChapterNumber,jobject jIncdec, jstring jCacheDir)
{
    const char* fileEPUB = env->GetStringUTFChars(jpath, nullptr);

    int incdec = integerToInt(env, jIncdec);
    int chapterNumber = integerHolderToInt(env, jChapterNumber);
    std::wstring wholeChapter;
    int amountOfPagesInChapter;
    int currentPage = integerHolderToInt(env, jCurrentPage);//TODO add to Java code
    double currentImage = doubleHolderToDouble(env, jCurrentImage);//TODO add to Java code
    int amountOfImages = 0;
    int previousChapterNumber = integerHolderToInt(env, jPreviousChapterNumber);//TODO add to Java code

    string tmpStrForCheck = fileEPUB;
    if (tmpStrForCheck.find("epub") != std::wstring::npos)
        wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
    if (tmpStrForCheck.find("fb2") != std::wstring::npos)
        wholeChapter = utf8_to_wstring(extractFB2(fileEPUB, chapterNumber, jCacheDir, env));

    int numberOfLines = 40;
    wstring text;
    std::vector<std::wstring> pages = returnPages(wholeChapter, numberOfLines, amountOfImages);
    text = pages[0];
    amountOfPagesInChapter = pages.size();
    bool showText = true;
    //string pathToImage = openingBook(chapterNumber, wholeChapter, numberOfLines, pages, currentPage, amountOfPagesInChapter, text, amountOfImages, fileEPUB, previousChapterNumber, currentImage, showText, 1000, 1000);
    changePage(chapterNumber, wholeChapter, numberOfLines, pages, currentPage, amountOfPagesInChapter, text, incdec, amountOfImages, fileEPUB, previousChapterNumber, currentImage, showText, 1000, 1000, jCacheDir, env);
    {//this block is used to change Java values
        changeJavaIntLinkTo(env, jChapterNumber, chapterNumber);
        changeJavaIntLinkTo(env, jCurrentPage, currentPage);
        changeJavaDoubleLinkTo(env, jCurrentImage, currentImage);
        changeJavaIntLinkTo(env, jPreviousChapterNumber, previousChapterNumber);
    }
    if (showText)
    {
        using convert_type = std::codecvt_utf8<wchar_t>;
        wstring_convert<convert_type, wchar_t> conventer;
        string tmp = conventer.to_bytes(text);
        return stdStringToJstring(env, tmp);
    }
    else
    {
        //return stdStringToJstring(env, readJPEG(fileEPUB, chapterNumber, currentImage, jCacheDir, env));
        if (tmpStrForCheck.find("epub") != std::wstring::npos)
            return stdStringToJstring(env, readJPEG(fileEPUB, chapterNumber, currentImage, jCacheDir, env));
        if (tmpStrForCheck.find("fb2") != std::wstring::npos)
        {
            int tmpN = currentImage;
            //string tmp = jstringToString(env, jCacheDir) + "/" + to_string(tmpN) + ".png";
            string tmp = jstringToString(env, jCacheDir) + "/" + std::to_string(tmpN) + ".png";
            return stdStringToJstring(env, tmp);
        }
    }
}
}