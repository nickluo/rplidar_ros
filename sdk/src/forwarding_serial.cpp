#include "forwarding_serial.h"

namespace rp { namespace arch { namespace net {

	forwarding_serial::forwarding_serial()
        : queue(0)
	{
		sender = n.advertise<std_msgs::UInt8MultiArray>("/rplidar_sender", 32);
		receiver = n.subscribe("rplidar_receiver", 32, 
			&forwarding_serial::arrayCallback, this);

        while(sender.getNumSubscribers()==0 || receiver.getNumPublishers()==0)
        {
            if (!ros::ok())
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

	}

	forwarding_serial::~forwarding_serial()
	{

	}

	void forwarding_serial::arrayCallback(const std_msgs::UInt8MultiArray::ConstPtr& ptr)
	{
        std::vector<_u8> v(ptr->data.size());
        std::memcpy(v.data(), ptr->data.data(), ptr->data.size());
        queue.Push(std::move(v));
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
        std::vector<_u8> *pv = queue.Peek(timeout);
        if (pv)
        {
            if (returned_size)
                *returned_size = pv->size();
            return ANS_OK;
        }
        if (returned_size)
            *returned_size = 0;
        return ANS_TIMEOUT;
    }

    int forwarding_serial::senddata(const unsigned char * data, size_t size)
    {
        std_msgs::UInt8MultiArray array;
        array.data.resize(size);
        std::memcpy(array.data.data(), data, size);
        //Publish array
        // if (sender.getNumSubscribers()==0)
        //     std::cout<<"publish fail "<<std::endl;
        sender.publish(array);
    	return size;
    }
    
    int forwarding_serial::recvdata(unsigned char * data, size_t size)
    {
    	if (!isOpened()) 
    		return 0;
        std::vector<_u8> v;
        if (queue.Pop_NoWait(v))
            std::memcpy(data, v.data(), size);
    	return 0;
    }

    int forwarding_serial::waitforsent(_u32 timeout, size_t * returned_size)
    {
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
