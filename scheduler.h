#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <map>
#include <string>
#include "request_defs.h"
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

class scheduler {
    private:
        std::map<int, model_state> srv_state;
        
        std::deque<cq_request> cq_queue;
        std::mutex cq_queue_mutex;
        std::condition_variable cq_queue_cv;

        std::thread cq_req_thread;
        
    public:
        scheduler();
        void schedule(int client_id, int model_id, std::string model_input, tcp_connection::pointer);
        void echo_input_back(int a, int b, std::string string_op, tcp_connection::pointer sp);
        void process_cq_request(void);         
};
#endif