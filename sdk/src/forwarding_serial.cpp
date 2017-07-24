#include "forwarding_serial.h"

namespace rp { namespace arch { namespace net {

	forwarding_serial::forwarding_serial()
	{
		sender = n.advertise<std_msgs::UInt8MultiArray>("rplidar_sender", 32);
		receiver = n.subscribe("rplidar_receiver", 32, 
			&forwarding_serial::arrayCallback, this);
	}

	forwarding_serial::~forwarding_serial()
	{

	}

	void forwarding_serial::arrayCallback(const std_msgs::UInt8MultiArray::ConstPtr& ptr)
	{
		std::unique_lock<std::mutex> lck(mtx);
		std::memcpy(rx_buffer, ptr->data.data(), ptr->data.size());
		required_rx_cnt = ptr->data.size();
		cv.notify_one();
	}

	void forwarding_serial::flush(_u32 flags)
	{
		(void)flags;
	}

    bool forwarding_serial::bind(const char * portname, _u32 baudrate, _u32 flags)
    {
    	(void)portname;
    	(void)baudrate;
    	(void)flags;
    	return true;
    }

    bool forwarding_serial::open()
    {
    	_is_serial_opened = true;
    	return true;
    }

    void forwarding_serial::close()
    {
    	_is_serial_opened = false;
    }
    
    int forwarding_serial::waitfordata(size_t data_count,_u32 timeout, size_t * returned_size)
    {
    	if (!isOpened())
    		return ANS_DEV_ERR;
    	std::unique_lock<std::mutex> lck(mtx);
  		if (cv.wait_for(lck, std::chrono::milliseconds(timeout),
  			[&](){ return required_rx_cnt>0; }))
  		{
  			if (returned_size)
  				*returned_size = required_rx_cnt;
  			return ANS_OK;
  		}
    	return ANS_TIMEOUT;
    }

    int forwarding_serial::senddata(const unsigned char * data, size_t size)
    {
    	std::memcpy(tx_buffer+required_tx_cnt, data, size);
    	required_tx_cnt += size;
    	return size;
    }
    
    int forwarding_serial::recvdata(unsigned char * data, size_t size)
    {
    	if (!isOpened()) 
    		return 0;
    	std::unique_lock<std::mutex> lck(mtx);
    	if (required_tx_cnt<size)
    		return -1;
    	std::memcpy(data, rx_buffer, required_tx_cnt);
    	required_tx_cnt = 0;
    	return 0;
    }

    int forwarding_serial::waitforsent(_u32 timeout, size_t * returned_size)
    {
    	std_msgs::UInt8MultiArray array;
    	array.data.resize(required_tx_cnt);
    	std::memcpy(array.data.data(), tx_buffer, required_tx_cnt);
    	//Publish array
		sender.publish(array);
    	if (returned_size)
    		*returned_size = required_tx_cnt;
    	return 0;
    }

    int forwarding_serial::waitforrecv(_u32 timeout, size_t * returned_size)
    {
    	return 0;
    }

    size_t forwarding_serial::rxqueue_count()
    {
    	return required_rx_cnt;
    }

    void forwarding_serial::setDTR() {}
    void forwarding_serial::clearDTR() {}

}}};
