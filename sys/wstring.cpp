#include <ntifs.h>
#include <ntdddisk.h>
#include <ntstrsafe.h>
#include "wstring.h"
#include "CppSupport.h"

// create unicodestring with null terminated
// Inline this function for showing position in leak analysis
inline UNICODE_STRING createUnicodeString2(const UNICODE_STRING &src) {
  UNICODE_STRING dst = src;
  auto maxLength = (src.Length == src.MaximumLength) ? (src.MaximumLength + 2)
                                                     : src.MaximumLength;
  dst.Buffer = (PWSTR)malloc(maxLength);
  dst.MaximumLength = (USHORT)maxLength;
  if (dst.Buffer)
    RtlCopyMemory(dst.Buffer, src.Buffer, src.Length);
  dst.Buffer[dst.Length / 2] = 0;
  return dst;
}

// created null terminated string, as RtlUnicodeStringInit already considered it
// Inline this function for showing position in leak analysis
inline UNICODE_STRING createUnicodeString(const PWSTR rawString) {
  UNICODE_STRING src;
  RtlUnicodeStringInit(&src, rawString);

  UNICODE_STRING dst = src;
  dst.Buffer = (PWSTR)malloc(src.MaximumLength);
  if (dst.Buffer)
    RtlCopyMemory(dst.Buffer, src.Buffer, src.MaximumLength);
  return dst;
}

//
// May generates non null-terminated string when clone is false
//
wstring::wstring(const UNICODE_STRING & origin, bool clone) {
    if (clone) {
        content = createUnicodeString2(origin);
    } else {
        // this path may create a non-null terminated string.
        content = origin;
    }
    ownBuffer = clone;
}

wstring::wstring(const UNICODE_STRING & origin) : wstring(origin, true) {
}

wstring::wstring(const PWSTR origin) {
    content = createUnicodeString(origin);
    ownBuffer = true;
}

wstring::wstring(const PWSTR origin, int bytesCount, bool clone) {
    if (clone) {
        content.Buffer = (WCHAR *)malloc(bytesCount);
        content.Length = (USHORT) bytesCount;
        content.MaximumLength = (USHORT) bytesCount;
        RtlCopyMemory(content.Buffer, origin, bytesCount);
    } else {
        content.Buffer = (WCHAR *)origin;
        content.Length = (USHORT) bytesCount;
        content.MaximumLength = (USHORT) bytesCount;
    }
    ownBuffer = clone;
}

wstring::wstring(int bytesCount) {
    if (bytesCount > 0) {
        content.Buffer = (WCHAR *)malloc(bytesCount);
        content.Length = 0;
        content.MaximumLength = (USHORT)bytesCount;
        ownBuffer = true;
    }
}

// Default copy constructor and will be called on every value return.
wstring::wstring(const wstring & other) : wstring(other.content, true) {
}

// Use header initializer
wstring::wstring() { }

wstring wstring::operator=(const wstring & rhs) {
    if (ownBuffer) free(content.Buffer);

    ownBuffer = true;
    content = createUnicodeString2(rhs.content);
    return *this;
}

wstring::~wstring() {
    if (ownBuffer) {
        free(content.Buffer);
    }
}

wstring wstring::clone() const {
    wstring cloned;
    cloned.content = createUnicodeString2(content);
    cloned.ownBuffer = true;
    return cloned;
}

wstring wstring::fastCopy() const {
    return wstring(content, false);
}

wstring wstring::toUpper() const {
    wstring result(content.MaximumLength);
    RtlUpcaseUnicodeString(&result.content, &content, false);
    return result;
}

int wstring::compare(const wstring & rhs) const {
    if (content.Length == 0 && rhs.content.Length == 0) return 0;
    return RtlCompareUnicodeString(&content, &rhs.content, false);
}

int wstring::icompare(const wstring & rhs) const {
    if (content.Length == 0 && rhs.content.Length == 0) return 0;
    auto result = RtlCompareUnicodeString(&content, &rhs.content, true);
    return result;
}

int wstring::findLast(WCHAR ch) const {
    if (content.Length == 0) return -1;

    int index = (content.Length / 2)  - 1;
    for (; index >= 0; index--) {
        if (content.Buffer[index] == ch) return index;
    }
    return index;
}

int wstring::index(const wstring & substr) const {
    if (content.Length == 0) return -1;
    if (substr.length() == 0) return -1;

    int i = 0;
    for (; i < (content.Length / 2); i++) {
        for (int j = 0; j < (substr.content.Length / 2); j++) {
            if (content.Buffer[i + j] != substr.content.Buffer[j]) break;
            if (j == ((substr.content.Length / 2) - 1)) return i;
        }
    }

    return -1;
}

bool wstring::startWith(const wstring & rhs) const {
    if (content.Length < rhs.content.Length) return false;
    for (int i = 0; i < (rhs.content.Length / 2); i++) {
        if (content.Buffer[i] != rhs.content.Buffer[i]) return false;
    }
    return true;
}

void * wstring::pointer() const {
    return content.Buffer;
}

unsigned wstring::size() const {
    return content.Length;
}

wstring *wstring::cloneByPointer() const {
    return new wstring(content, true);
}
