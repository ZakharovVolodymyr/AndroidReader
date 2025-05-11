#pragma once


std::string jstringToString(JNIEnv* env, jstring jStr) {
    if (!jStr)
        return "";

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray)env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t)env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char*)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

int integerToInt(JNIEnv* env, jobject chapter)
{
    jclass integerClass = env->GetObjectClass(chapter);
    jmethodID intValueMethod = env->GetMethodID(integerClass, "intValue", "()I");
    int chapterValue = env->CallIntMethod(chapter, intValueMethod);
    return chapterValue;
}

jstring stdStringToJstring(JNIEnv* env, const std::string& str) {
    jstring jStr = env->NewStringUTF(str.c_str());
    return jStr;
}

void changeJavaIntLinkTo(JNIEnv* env, jobject obj, int FinalValue) {
    jclass integerHolderClass = env->GetObjectClass(obj);
    if (integerHolderClass == nullptr) {
        return;
    }
    jfieldID valueFieldID = env->GetFieldID(integerHolderClass, "value", "I");
    if (valueFieldID == nullptr) {
        return;
    }
    jint currentValue = env->GetIntField(obj, valueFieldID);
    jint newValue = FinalValue;
    env->SetIntField(obj, valueFieldID, newValue);
}

void changeJavaDoubleLinkTo(JNIEnv* env, jobject obj, double FinalValue) {
    jclass doubleHolderClass = env->GetObjectClass(obj);
    if (doubleHolderClass == nullptr) {
        return;
    }
    jfieldID valueFieldID = env->GetFieldID(doubleHolderClass, "value", "D");
    if (valueFieldID == nullptr) {
        return;
    }
    jdouble  currentValue = env->GetDoubleField(obj, valueFieldID);
    jdouble newValue = FinalValue;
    env->SetDoubleField(obj, valueFieldID, newValue);
}

int integerHolderToInt(JNIEnv* env, jobject chapter)
{
    jclass integerHolderClass = env->GetObjectClass(chapter);
    jfieldID valueFieldID = env->GetFieldID(integerHolderClass, "value", "I");
    jint currentValue = env->GetIntField(chapter, valueFieldID);
    int valueToReturn = currentValue;
    return valueToReturn;
}

double doubleHolderToDouble(JNIEnv* env, jobject chapter)
{
    jclass doubleHolderClass = env->GetObjectClass(chapter);
    jfieldID valueFieldID = env->GetFieldID(doubleHolderClass, "value", "D");
    jdouble currentValue = env->GetDoubleField(chapter, valueFieldID);
    double valueToReturn = currentValue;
    return valueToReturn;
}