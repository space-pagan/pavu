#include <pulse/pulseaudio.h>
#include <queue>
#include <string>

struct Monitor{
        std::string stream_name; // the stream name to search for
        uint32_t stream_index; // the index to search for
        uint32_t rate; // polling rate in Hz
        std::queue<int> q; // stream peak data queue
        bool is_sink; // is the target stream a sink or a source?
        bool is_connected = false; // have we found the target stream?
        
        Monitor(std::string stream_name, uint32_t rate);
        Monitor(uint32_t index, bool is_sink, uint32_t rate);

    private:
        void pa_mainloop_init();
        static void context_notify_cb(pa_context* c, void* userdata);
        static void sink_source_info_cb(
                pa_context *c, const pa_sink_info *i,
                const pa_source_info *j, void* userdata);
        static void sink_info_cb(
                pa_context *c, const pa_sink_info *i, 
                int eol, void* userdata);
        static void source_info_cb(
                pa_context *c, const pa_source_info *i, 
                int eol, void* userdata);
        static void stream_rq_cb(
                pa_stream* stream, size_t nbytes, void* userdata);
};
