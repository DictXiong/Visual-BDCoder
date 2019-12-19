#pragma once
#include<utility>
#include<vector>

constexpr short H264 = 0;
constexpr short H265 = 1;

constexpr short PASS2 = 0;
constexpr short CRF = 1;
constexpr short VBR = 2;

constexpr int CODED = 0;
constexpr int READY = 1;
constexpr int CODING = 2;

constexpr short MULTI_VALUE = -1;

class InputFile
{
  private:
    CString path,dir;
    int vb, ab, format, code, crf;
    short cut, acopy, vcopy, ready = READY;
    std::pair<int, int> range;
  public:
    InputFile(CString path, int format, int code, int vb, int ab, bool vcopy, bool acopy);
    InputFile(CString path, InputFile settings);
    std::vector<CString> translate(CString outputPath = L"");
    int setVideoBitrate(int vb);
    int getVideoBitrate();
    int setAudioBitrate(int ab);
    int getStatus();
    int setStatus(int status);
    bool setCut(bool cut);
    int setCode(int code);
    int setFormat(int format);
    bool setCopyVideoStream(bool vcopy);
    bool setCopyAudioStream(bool acopy);
    CString getPath();
    std::vector<int> getData();
    bool setData(std::vector<int> data, bool writeNegativeOne = true);
    bool setData(InputFile data, bool writeNegativeOne = true, bool overrideCRFOrVB=false);
    std::pair<int, int> setCutRange(int start, int end);
    std::pair<int, int> setCutRange(std::pair<int, int> range);
    std::vector<CString> getItemInfo();
};

