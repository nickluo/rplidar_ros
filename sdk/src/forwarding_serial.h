#pragma once

#include <cstring>
#include <condition_variable>
#include <mutex>
#include <thread>

using size_t = std::size_t;

#include "hal/abs_rxtx.h"

#include "ros/ros.h"

#include "std_msgs/MultiArrayLayout.h"
#include "std_msgs/MultiArrayDimension.h"
#include "std_msgs/UInt8MultiArray.h"

namespace rp { namespace arch { namespace net {

class forwarding_serial : public rp::hal::serial_rxtx
{
public:
	forwarding_serial();
	virtual ~forwarding_serial();

	virtual void flush(_u32 flags) override;

    virtual bool bind(const char * portname, _u32 baudrate, _u32 flags = 0) override;
    virtual bool open() override;
    virtual void close() override;
    
    virtual int waitfordata(size_t data_count,_u32 timeout = -1, 
    	size_t * returned_size = NULL) override;

    virtual int senddata(const unsigned char * data, size_t size) override;
    virtual int recvdata(unsigned char * data, size_t size) override;

    virtual int waitforsent(_u32 timeout = -1, size_t * returned_size = NULL) override;
    virtual int waitforrecv(_u32 timeout = -1, size_t * returned_size = NULL) override;

    virtual size_t rxqueue_count() override;

    virtual void setDTR() override;
    virtual void clearDTR() override;
protected:
	size_t required_tx_cnt = 0;
	size_t required_rx_cnt = 0;
	unsigned char tx_buffer[256];
	unsigned char rx_buffer[256];
	ros::NodeHandle n;
	ros::Publisher sender;
	ros::Subscriber receiver;
	std::mutex mtx;
	std::condition_variable cv;

	void arrayCallback(const std_msgs::UInt8MultiArray::ConstPtr& ptr);
};

}}}