#include "H264File.h"
#include <cstring>

H264File::H264File(int buf_size) : m_buf_size(buf_size)
{
    m_buf = new char[m_buf_size];
}

H264File::~H264File()
{
    delete[] m_buf;
}

bool H264File::Open(const char *path)
{
    m_file = fopen(path, "rb");
    return m_file != NULL;
}

void H264File::Close()
{
    if (m_file)
    {
        fclose(m_file);
        m_file = NULL;
        m_count = 0;
        m_bytes_used = 0;
    }
}

int H264File::ReadFrame(char *in_buf, int in_buf_size, bool *end)
{
    if (!m_file)
        return -1;

    int bytes_read = (int)fread(m_buf, 1, m_buf_size, m_file);
    if (bytes_read == 0)
    {
        fseek(m_file, 0, SEEK_SET);
        m_count = 0;
        m_bytes_used = 0;
        bytes_read = (int)fread(m_buf, 1, m_buf_size, m_file);
        if (bytes_read == 0)
        {
            this->Close();
            return -1;
        }
    }

    bool is_find_start = false;
    int i = 0;
    for (i = 0; i < bytes_read - 5; i++)
    {
        if (m_buf[i] == 0 && m_buf[i + 1] == 0 && m_buf[i + 2] == 0 && m_buf[i + 3] == 1)
        {
            is_find_start = true;
            i += 4;
            break;
        }
        else if (m_buf[i] == 0 && m_buf[i + 1] == 0 && m_buf[i + 2] == 1)
        {
            is_find_start = true;
            i += 3;
            break;
        }
    }
    if (!is_find_start)
    {
        this->Close();
        return -1;
    }

    bool is_find_end = false;
    int start_code = 0;
    for (int j = i; j < bytes_read - 5; j++)
    {
        if (m_buf[j] == 0 && m_buf[j + 1] == 0 && m_buf[j + 2] == 0 && m_buf[j + 3] == 1)
        {
            start_code = 4;
            is_find_end = true;
            memcpy(in_buf, m_buf + i - start_code, j - i + start_code);
            m_bytes_used += j;
            fseek(m_file, m_bytes_used, SEEK_SET);
            return j - i + start_code;
        }
        else if (m_buf[j] == 0 && m_buf[j + 1] == 0 && m_buf[j + 2] == 1)
        {
            start_code = 3;
            is_find_end = true;
            memcpy(in_buf, m_buf + i - start_code, j - i + start_code);
            m_bytes_used += j;
            fseek(m_file, m_bytes_used, SEEK_SET);
            return j - i + start_code;
        }
    }

    // 如果到文件尾部还没有找到下一个起始码，则认为当前帧是最后一帧
    if (!is_find_end)
    {
        int frame_size = bytes_read - (i - start_code);
        if (frame_size > in_buf_size)
        {
            this->Close();
            return -1;
        }
        memcpy(in_buf, m_buf + i - start_code, frame_size);
        m_bytes_used += bytes_read;
        fseek(m_file, m_bytes_used, SEEK_SET);
        return frame_size;
    }

    return -1;
}