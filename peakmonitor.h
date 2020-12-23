#include <pulse/pulseaudio.h>
#include <queue>
#include <string>

struct PeakMonitor{
        std::string sink_name;
        uint32_t sink_index;
        uint32_t rate;
        std::queue<int> q;
        
        PeakMonitor(std::string sink_name, uint32_t rate);
        PeakMonitor(uint32_t index, uint32_t rate);
        static void context_notify_cb(pa_context* c, void* userdata);
        static void sink_info_cb(
                pa_context *c, const pa_sink_info *i, 
                int eol, void* userdata);
        static void stream_rq_cb(
                pa_stream* stream, size_t nbytes, void* userdata);

    private:
        void pa_mainloop_init();
};

