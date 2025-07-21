#include <iostream>
class H264File
{
public:
    H264File(int buf_size = 500000);
    ~H264File();

    bool Open(const char *path);
    void Close();

    bool IsOpened() const
    {
        return (m_file != NULL);
    }

    int ReadFrame(char *in_buf, int in_buf_size, bool *end);

private:
    FILE *m_file = NULL;
    char *m_buf = NULL;
    int m_buf_size = 0;
    int m_bytes_used = 0;
    int m_count = 0;
};