#include "pch.h"
#include "InputFile.h"
#include<afxstr.h>
#include<utility>
#include<vector>



InputFile::InputFile(CString path, int format, int code, int vb, int ab, bool vcopy, bool acopy)
{
    this->path = path;
    int splitPos = path.ReverseFind(L'\\');
    if (splitPos != -1) this->dir = path.Left(splitPos);
    this->format = format;
    this->code = code;
    if (code == CRF)
    {
        this->crf = vb;
        this->vb = 5000;
    }
    else
    {
        this->vb = vb;
        this->crf = 25;
    }
    this->ab = ab;
    this->cut = false;
    this->acopy = acopy;
    this->vcopy = vcopy;
    this->range = std::make_pair(0, 0);
    
}

InputFile::InputFile(CString path, InputFile settings)
{
    this->path = path;
    int splitPos = path.ReverseFind(L'\\');
    if (splitPos != -1) this->dir = path.Left(splitPos);
    this->setData(settings.getData());
    if (code == CRF)
    {
        vb = 5000;
    }
    else
    {
        crf = 25;
    }
}

std::vector<CString> InputFile::translate(CString outputPath)
{
    auto combineArgs = [](std::vector<CString> args)->CString
    {
        CString ans;
        for (auto i : args)
        {
            ans += i + L" ";
        }
        return ans;
    };

    const CString common = L"-keyint_min 30 -preset slow -f mp4 -hide_banner -y";

    CString tmp, after=L"_bdcoder.mp4";
    std::vector<CString> ans;
    std::vector<CString> commands;
    ans.push_back(L"ffmpeg");

    if (cut)
    {
        tmp.Format(L"-ss %d -to %d -accurate_seek", range.first, range.second);
        ans.push_back(tmp);
        after = L"_bdcoder_cut.mp4";
    }

    ans.push_back(L"-i \"" + path + L"\"");
    if (cut) ans.push_back(L"-avoid_negative_ts 1");
    ans.push_back(common);

    ans.push_back(L"-vcodec");
    if (vcopy)
    {
        ans.push_back(L"copy");
    }
    else
    {
        if (format == H264)
        {
            if (code == VBR) ans.push_back(L"h264_nvenc");
            else ans.push_back(L"libx264");
            ans.push_back(L"-profile:v high10");
        }
        else
        {
            if (code == VBR) ans.push_back(L"hevc_nvenc");
            else ans.push_back(L"libx265");
            ans.push_back(L"-profile:v main10");
        }

        if (code == CRF)
        {
            ans.push_back(L"-crf");
            tmp.Format(L"%d", crf);
            ans.push_back(tmp);

        }

        if (code == PASS2)
        {
            ans.push_back(L"-b:v");
            tmp.Format(L"%dk", vb);
            ans.push_back(tmp);
            auto p2 = ans;
            p2.push_back(L"-pass 1");
            ans.push_back(L"-pass 2");
            p2.push_back(L"-an NUL");
            commands.push_back(combineArgs(p2));
            //ans.insert(ans.end(), p2.begin(), p2.end());
        }
    }

    // after
    if (!acopy) tmp.Format(L"-acodec aac -b:a %dki", ab);
    else tmp = L"-acodec copy";
    ans.push_back(tmp);
    if (outputPath == L"")
    {
        outputPath = dir;
    }
    if (outputPath[outputPath.GetLength() - 1] != L'\\')
    {
        outputPath += L'\\';
    }
    int splitPos = path.ReverseFind(L'\\');
    if (splitPos != -1) tmp = path.Right(path.GetLength() - splitPos-1);
    else tmp = path;
    splitPos = tmp.ReverseFind(L'.');
    if (splitPos!=-1) tmp= tmp.Left(splitPos);
    ans.push_back(L"\"" + outputPath+ tmp + L"_vbdcoder.mp4\"");

    // combine
    CString command;
    for (auto i : ans)
    {
        command += i + L" ";
    }
    commands.push_back(command);
    return commands;
}

int InputFile::setVideoBitrate(int vb)
{
    this->ready = true;

    int tmp;
    if (code == CRF)
    {
        tmp = this->crf;
        this->crf = vb;
    }
    else
    {
        tmp = this->vb;
        this->vb = vb;
    }
    return tmp;
}

int InputFile::getVideoBitrate()
{
    if (code == CRF) return crf;
    else return vb;
}

int InputFile::setAudioBitrate(int ab)
{
    this->ready = true;

    int tmp = this->ab;
    this->ab = ab;
    return tmp;
}

int InputFile::getStatus()
{
    return ready;
}

int InputFile::setStatus(int status)
{
    this->ready = true;

    auto tmp = ready;
    ready = status;
    return tmp;
}

bool InputFile::setCut(bool cut)
{
    this->ready = true;

    bool tmp = this->cut;
    this->cut = cut;
    return tmp;
}

int InputFile::setCode(int code)
{
    this->ready = true;

    int tmp = this->code;
    this->code = code;
    return tmp;
}

int InputFile::setFormat(int format)
{
    this->ready = true;

    int tmp = this->format;
    this->format = format;
    return tmp;
}

std::pair<int, int> InputFile::setCutRange(int start, int end)
{
    this->ready = true;

    auto tmp = this->range;
    this->range = std::make_pair(start, end);
    return tmp;
}

std::pair<int, int> InputFile::setCutRange(std::pair<int, int> range)
{
    this->ready = true;

    auto tmp = this->range;
    this->range = range;
    return tmp;
}

bool InputFile::setCopyVideoStream(bool vcopy)
{
    this->ready = true;

    auto tmp = this->vcopy;
    this->vcopy = vcopy;
    return tmp;
}

bool InputFile::setCopyAudioStream(bool acopy)
{
    this->ready = true;

    auto tmp = this->acopy;
    this->acopy = acopy;
    return tmp;
}

CString InputFile::getPath()
{
    return path;
}

std::vector<CString> InputFile::getItemInfo()
{
    std::vector<CString> ans;
    ans.push_back(path);
    switch (ready)
    {
    case READY: ans.push_back(L"就绪"); break;
    case CODING: ans.push_back(L"转码中"); break;
    case CODED: ans.push_back(L"完成"); break;
    }
    switch (format)
    {
    case H264: ans.push_back(L"H264"); break;
    case H265: ans.push_back(L"H265"); break;
    }
    switch (code)
    {
    case PASS2: ans.push_back(L"2P"); break;
    case CRF: ans.push_back(L"CRF"); break;
    case VBR: ans.push_back(L"VBR"); break;
    }
    CString tmp;
    if (!vcopy)
    {
        if (code==CRF) tmp.Format(L"%d", crf);
        else tmp.Format(L"%d", vb);
    }
    else tmp = L"copy";
    ans.push_back(tmp);

    if (!acopy) tmp.Format(L"%d", ab);
    else tmp = L"copy";
    ans.push_back(tmp);

    if (cut) 
    {
        tmp.Format(L"%02d:%02d:%02d", range.first / 3600, range.first % 3600 / 60, range.first % 60);
        ans.push_back(tmp);
        tmp.Format(L"%02d:%02d:%02d", range.second / 3600, range.second % 3600 / 60, range.second % 60);
        ans.push_back(tmp);
    }
    else
    {
        ans.push_back(L"-");
        ans.push_back(L"-");
    }
    return ans;
}

std::vector<int> InputFile::getData()
{
    if (code != CRF)
    {
        std::vector<int> ans{ format,code,vb,ab,vcopy,acopy,cut,range.first,range.second };
        return ans;
    }
    else
    {
        std::vector<int> ans{ format,code,crf,ab,vcopy,acopy,cut,range.first,range.second };
        return ans;
    }
}

bool InputFile::setData(std::vector<int> data, bool writeNegativeOne)
{
    this->ready = true;

    if (writeNegativeOne || data[0] != -1) this->format = data[0];
    if (writeNegativeOne || data[1] != -1) this->code = data[1];
    if (writeNegativeOne || data[2] != -1) 
    {
        if (data[1] != CRF)
        {
            this->vb = data[2];
            //this->crf = 25;
        }
        else
        {
            this->crf = data[2];
            //this->vb = 5000;
        }
    }
    if (writeNegativeOne || data[3] != -1)this->ab = data[3];
    if (writeNegativeOne || data[4] != -1)this->vcopy = data[4];
    if (writeNegativeOne || data[5] != -1)this->acopy = data[5];
    if (writeNegativeOne || data[6] != -1)this->cut = data[6];
    if (writeNegativeOne || data[7] != -1)this->range.first = data[7];
    if (writeNegativeOne || data[8] != -1)this->range.second = data[8];
    return false;
}

bool InputFile::setData(InputFile data, bool writeNegativeOne, bool overrideCRFOrVB)
{
    this->ready = true;

    if (writeNegativeOne || data.format != -1) this->format = data.format;
    if (writeNegativeOne || data.code != -1) this->code = data.code;
    if (writeNegativeOne || data.vb != -1)
    {
        if (data.code != CRF)
        {
            this->vb = data.vb;
            if (overrideCRFOrVB) this->crf = data.crf;
        }
        else
        {
            this->crf = data.crf;
            if (overrideCRFOrVB) this->vb = data.vb;
        }
    }
    if (writeNegativeOne || data.ab != -1)this->ab = data.ab;
    if (writeNegativeOne || data.vcopy != -1)this->vcopy = data.vcopy;
    if (writeNegativeOne || data.acopy != -1)this->acopy = data.acopy;
    if (writeNegativeOne || data.cut != -1)this->cut = data.cut;
    if (writeNegativeOne || data.range.first != -1)this->range.first = data.range.first;
    if (writeNegativeOne || data.range.second != -1)this->range.second = data.range.second;
    return false;
}
