#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include "scheduler.h"

scheduler::scheduler() : 
    cq_queue_mutex(),
    cq_queue_cv(),

    cq_req_thread(&scheduler::process_cq_request, this){
    srv_state.insert(std::make_pair(1, model_state(1,300,100)));
}

void scheduler::schedule(int client_id, int model_id, std::string model_input, tcp_connection::pointer sp){

    std::unique_lock<std::mutex> cq_mutex_mgr(cq_queue_mutex);
    std::cout<<"inside make_nference, acquiring the lock now "<<std::endl;
    auto nr = cq_request(client_id, model_id, model_input, sp);
    cq_queue.push_back(nr);
    cq_mutex_mgr.unlock();
    cq_queue_cv.notify_one();   
}

void scheduler::process_cq_request(void){
    while(1){

        std::unique_lock<std::mutex> cq_mutex_mgr(cq_queue_mutex);
        cq_queue_cv.wait(cq_mutex_mgr, [this](){ return cq_queue.size() != 0; });
        auto nr = cq_queue.back();
        cq_queue.pop_back();
        cq_mutex_mgr.unlock();

        std::cout<<"inside the process network request thread processing request from client "<<nr.client_id<<std::endl;

        auto x = srv_state.find(nr.model_id);
        std::this_thread::sleep_for(std::chrono::milliseconds( (x->second).cpu_prep_time ));
        echo_input_back(nr.client_id, nr.model_id, nr.model_input, nr.connection_sp);
    }
}

void scheduler::echo_input_back(int client_id, int model_id, std::string op_string, tcp_connection::pointer sp){
    msgpack::sbuffer sbuf;
	msgpack::packer<msgpack::sbuffer> packer(sbuf);
	packer.pack_array(2);
    int64_t response_type = 1;
    packer.pack_int(response_type);
    packer.pack_str(op_string.size());
    packer.pack_str_body(op_string.c_str(), op_string.size());
    msgpack::object_handle result;
    msgpack::unpack(result, sbuf.data(), sbuf.size());
    
    std::cout<<result.get()<<std::endl;   
	string outBuff(sbuf.data(), sbuf.size());
	cout<<"echoeing input string back "<<endl;
    boost::asio::write(sp->socket(), boost::asio::buffer(outBuff));

}