class AACFile
{
public:
    AACFile() = default;
    ~AACFile() { Close(); }

    bool Open(const char *path)
    {
        m_file = fopen(path, "rb");
        return m_file != nullptr;
    }

    void Close()
    {
        if (m_file)
        {
            fclose(m_file);
            m_file = nullptr;
        }
    }

    // 读取一个ADTS帧
    bool ReadFrame(std::vector<uint8_t> &out_frame)
    {
        if (!m_file)
            return false;

        uint8_t adts_header[7];
        size_t bytes_read = fread(adts_header, 1, 7, m_file);
        if (bytes_read < 7)
        {
            fseek(m_file, 0, SEEK_SET); // 文件结束, 回到开头
            bytes_read = fread(adts_header, 1, 7, m_file);
            if (bytes_read < 7)
                return false; // 文件太小
        }

        // 检查同步字 0xFFF
        if (adts_header[0] != 0xFF || (adts_header[1] & 0xF0) != 0xF0)
        {
            Close();
            return false;
        }

        // 从ADTS头中解析帧长度
        int frame_length = ((adts_header[3] & 0x03) << 11) | (adts_header[4] << 3) | ((adts_header[5] & 0xE0) >> 5);
        if (frame_length <= 7)
            return false;

        out_frame.resize(frame_length);
        memcpy(out_frame.data(), adts_header, 7);

        size_t frame_body_read = fread(out_frame.data() + 7, 1, frame_length - 7, m_file);
        if (frame_body_read != frame_length - 7)
        {
            fseek(m_file, 0, SEEK_SET);
            return false;
        }

        return true;
    }

private:
    FILE *m_file = nullptr;
};