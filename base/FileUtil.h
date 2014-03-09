#ifndef BASE_FILEUTIL_H
#define BASE_FILEUTIL_H
#include <base/StringPiece.h>
#include <boost/noncopyable.hpp>
#include <string.h>

namespace leaf {

namespace FileUtil {
  class SmallFile : boost::noncopyable {
  public:
    SmallFile(StringPiece filename);
    ~SmallFile();

    // return errno
    template<typename String>
      int readToString(int maxSize,
		       String* content,
		       int64_t* fileSize,
		       int64_t* modifyTime,
		       int64_t* createTime);

    /// Read ad maxium kBufferSize into buf_
    // return errno
    int readToBuffer(int* size);

    const char* buffer() const { return buf_; }

    static const int kBufferSize = 65536;
  private:
    int fd_;
    int err_;
    char buf_[kBufferSize];
  };

  // read the file content, returns errno if error happens.
  template<typename String>
    int readFile(StringPiece filename,
		 int maxSize,
		 String* content,
		 int64_t* fileSize = NULL,
		 int64_t* modifyTime = NULL,
		 int64_t* createTime = NULL) {
    SmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
  }
    
}

}



#endif // BASE_FILEUTIL_H
