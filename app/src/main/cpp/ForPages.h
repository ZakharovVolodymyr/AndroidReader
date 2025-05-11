#pragma once

bool returnTextOrImage(int& currentPage, std::vector<std::wstring>& pages, wstring& textToReturn)
{
	if (pages[currentPage].find(L"<img>") == std::wstring::npos && pages[currentPage].find(L".png") == std::wstring::npos)
	{
		textToReturn = pages[currentPage];
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<std::wstring> returnPages(std::wstring wholeChapter, int numberOfLines, int& amountOfImages)
{
	std::wstringstream wholeChapterSS;
	wholeChapterSS << wholeChapter;
	std::vector<std::wstring> pagesToReturn;

	int pageNumber = 0;;

	while (wholeChapterSS) 
	{
		std::wstring tmp = L"";
		for (int i = 0; i < numberOfLines && wholeChapterSS; i++)
		{
			std::wstring megatmp;
			getline(wholeChapterSS, megatmp);
			if (megatmp.find(L"<img>") != std::wstring::npos)
			{
				if(tmp != L"")
					pagesToReturn.push_back(tmp);
				tmp = megatmp;
				i = numberOfLines + 1;
				amountOfImages++;
			}
			else
			{
				tmp += megatmp;
				tmp += '\n';
			}
		}
		if (hasLettersW(tmp)) { //it dosnt work because not all images are processed
			pagesToReturn.push_back(tmp);
			pageNumber++;
		}
	}
	return pagesToReturn;
}

bool changePage(int& chapterNumber, std::wstring& wholeChapter, int& numberOfLines, std::vector<std::wstring>& pages, int& currentPage,
        int& amountOfPagesInChapter, wstring& textToReturn, int incdec, int& amountOfImages, string fileEPUB,
	    int& previousChapterNumber, double& currentImage, bool& showText, int Width, int Height, jstring jCacheDir, JNIEnv* env)
{
	bool boolToReturn;
	currentPage += incdec;
	if (currentPage == amountOfPagesInChapter || currentPage == -1) 
	{
		chapterNumber += incdec;
        if (fileEPUB.find("epub") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
        if (fileEPUB.find("fb2") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractFB2(fileEPUB, chapterNumber, jCacheDir, env));

		if (wholeChapter == L"<end>")
		{
			chapterNumber -= incdec;
			currentPage -= incdec;

            if (fileEPUB.find("epub") != std::wstring::npos)
                wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
            if (fileEPUB.find("fb2") != std::wstring::npos)
                wholeChapter = utf8_to_wstring(extractFB2(fileEPUB, chapterNumber, jCacheDir, env));
		}
		else 
		{
			amountOfImages = 0;
			pages = returnPages(wholeChapter, numberOfLines, amountOfImages);
			amountOfPagesInChapter = pages.size();

			(incdec > 0) ? currentPage = 0 : currentPage = amountOfPagesInChapter - 1;//-1 because amountOfPagesInChapter is amount of pages but it starts counting from 0

			boolToReturn = returnTextOrImage(currentPage, pages, textToReturn);
		}
	}
	else
	{
		boolToReturn = returnTextOrImage(currentPage, pages, textToReturn);
	}


	if (!boolToReturn)
	{
		if (previousChapterNumber != chapterNumber)
		{
			previousChapterNumber = chapterNumber;
            if (fileEPUB.find("epub") != std::wstring::npos)
			    (incdec > 0) ? currentImage = -1 : currentImage = amountOfImages;
		}
		int tmpInt = static_cast<int>(currentImage);
		if (incdec > 0)
		{
			currentImage = tmpInt;
		}
		else if (currentImage > tmpInt)
		{
			currentImage = tmpInt + 1;
		}
		currentImage += incdec;
        if (fileEPUB.find("epub") != std::wstring::npos)
            readJPEG(fileEPUB, chapterNumber, currentImage, jCacheDir, env);

		showText = false;
	}
	else
	{
		showText = true;
		if(incdec > 0)
			currentImage += 0.0001;
		else
			currentImage -= 0.0001;
		if (previousChapterNumber != chapterNumber)
		{
			previousChapterNumber = chapterNumber;
            if (fileEPUB.find("epub") != std::wstring::npos)
			    (incdec > 0) ? currentImage = -1 : currentImage = amountOfImages;
		}
	}
	return boolToReturn;
}

bool changePage(int& chapterNumber, std::wstring& wholeChapter, int& numberOfLines, std::vector<std::wstring>& pages, int& currentPage, int& amountOfPagesInChapter, wstring& textToReturn, int incdec, int& amountOfImages, string fileEPUB, jstring jCacheDir, JNIEnv* env)
{
	currentPage += incdec;
	if (currentPage == amountOfPagesInChapter || currentPage == -1)
	{
		chapterNumber += incdec;

        if (fileEPUB.find("epub") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
        if (fileEPUB.find("fb2") != std::wstring::npos)
            wholeChapter = utf8_to_wstring(extractFB2(fileEPUB, chapterNumber, jCacheDir, env));

		if (wholeChapter == L"<end>")
		{
			chapterNumber -= incdec;
			currentPage -= incdec;
			wholeChapter = utf8_to_wstring(extractEPUB(fileEPUB, chapterNumber));
		}
		else
		{
			amountOfImages = 0;
			pages = returnPages(wholeChapter, numberOfLines, amountOfImages);
			amountOfPagesInChapter = pages.size();

			(incdec > 0) ? currentPage = 0 : currentPage = amountOfPagesInChapter - 1;//-1 because amountOfPagesInChapter is amount of pages but it starts counting from 0

			return returnTextOrImage(currentPage, pages, textToReturn);
		}
	}
	else
	{
		return returnTextOrImage(currentPage, pages, textToReturn);
	}
}

string openingBook(int& chapterNumber, std::wstring& wholeChapter, int& numberOfLines, std::vector<std::wstring>& pages,
    int& currentPage,int& amountOfPagesInChapter, wstring& textToReturn, int& amountOfImages, string fileEPUB,
	int& previousChapterNumber, double& currentImage, bool& showText, int Width, int Height, jstring jCacheDir, JNIEnv* env)
{
	if (!changePage(chapterNumber, wholeChapter, numberOfLines, pages, currentPage, amountOfPagesInChapter, textToReturn, 1, amountOfImages, fileEPUB, jCacheDir, env));
	if (!changePage(chapterNumber, wholeChapter, numberOfLines, pages, currentPage, amountOfPagesInChapter, textToReturn, -1, amountOfImages, fileEPUB, jCacheDir, env))
	{
		currentImage = 0;
		showText = false;
        if (fileEPUB.find("epub") != std::wstring::npos)
		    return readJPEG(fileEPUB, chapterNumber, currentImage, jCacheDir, env);
        if (fileEPUB.find("fb2") != std::wstring::npos)
        {
            int tmpN = currentImage + 1;
            string tempImagePath = jstringToString(env, jCacheDir) + "/" + to_string(tmpN) + ".png";
            return tempImagePath;
        }
	}
	else
	{
		showText = true;
		return "";
	}
}