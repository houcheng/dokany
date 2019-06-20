#pragma once
#include <fltKernel.h>
#include "hash.h"

class NONPAGESECTION wstring : public Key {
    bool ownBuffer = false;
public:
    UNICODE_STRING content = { };

    //
    // Default constructor, default copy constructor and assignment operator
    //
    wstring();
    // Called on every function value return
    wstring(const wstring &other);
    wstring operator=(const wstring &rhs);

    virtual ~wstring();

    wstring(const UNICODE_STRING &origin);
    wstring(const UNICODE_STRING &origin, bool clone);

    wstring(const PWSTR origin);
    wstring(const PWSTR, int bytesCount, bool clone);

    wstring(int bytesCount);

    wstring clone() const;
    wstring fastCopy() const;
    wstring toUpper() const;

    int compare(const wstring &rhs) const;
    int icompare(const wstring &rhs) const;
    int findLast(WCHAR ch) const;
    int index(const wstring &substr) const;
    int length() const { return content.Length; }
    bool isEmptyOrNull() const { return (content.Length == 0) || (content.Buffer == nullptr); }
    bool isOwnBuffer() const{ return ownBuffer; }
    bool startWith(const wstring &rhs) const;

    //
    // Key interface
    //
    virtual void *pointer() const;
    virtual unsigned size() const;
    virtual wstring *cloneByPointer() const;
};

inline bool operator==(const wstring& lhs, const wstring& rhs) {
    return lhs.compare(rhs) == 0;
}

inline bool operator!=(const wstring& lhs, const wstring& rhs) {
    return lhs.compare(rhs) != 0;
}

inline bool operator%=(const wstring& lhs, const wstring& rhs) {
    return lhs.icompare(rhs) == 0;
}

inline bool operator==(const wstring& lhs, const PWSTR rhs) {
    return lhs.compare(wstring(rhs)) == 0;
}

inline bool operator!=(const wstring& lhs, const PWSTR rhs) {
    return lhs.compare(wstring(rhs)) != 0;
}

inline bool operator%=(const wstring& lhs, const PWSTR rhs) {
    return lhs.icompare(wstring(rhs)) == 0;
}


inline wstring operator+(const wstring& lhs, const wstring& rhs) {
    wstring sum(lhs.content.Length + rhs.content.Length);
    sum.content.Length = lhs.content.Length + rhs.content.Length;
    RtlCopyMemory(sum.content.Buffer, lhs.content.Buffer, lhs.content.Length);
    auto index = lhs.content.Length / 2;
    RtlCopyMemory(sum.content.Buffer + index, rhs.content.Buffer, rhs.content.Length);
    return sum;
}

inline wstring operator+(const wstring& lhs, const PWSTR rhs) {
    return lhs + wstring(rhs);
}

// Returns wstring with null terminated content
inline wstring operator~(const wstring&lhs) {
    if (lhs.content.MaximumLength > lhs.content.Length) {
        lhs.content.Buffer[lhs.content.Length / 2] = 0;
        return lhs;
    }

    wstring result(lhs.content, true);
    return result;
}
