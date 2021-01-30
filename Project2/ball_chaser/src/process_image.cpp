#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO("Driving the robot towards- j1:%1.2f, j2:%1.2f", lin_x, ang_z);

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested moving direction
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    int ball_pos = -1;

    // Loop through each pixel in the image and check if its contains a white pixel representing white ball
    for (int i = 0; i < img.height; i++) {
    	for (int j = 0; j < img.step; j++) {
    		int pos = i * img.step + j;
    		if (img.data[pos] == white_pixel) {
	            ball_pos = j;
	            break;
        	}
    	}
    }

    // In case no white ball in the image stop the robot
    if (ball_pos == -1)
    {
    	drive_robot(0.0, 0.0);
	return;
    }

    bool left, mid, right;
    left = mid = right = false;
    // Identify if the white pixel falls in the left, mid or right
    if (ball_pos < img.step/3)
	left = true;
    else if (ball_pos >= (img.step*2)/3)
	right = true;
    else
	mid = true;

    if (left)
    {
    	// Move left
    	drive_robot(0.0, 0.5);
    }
    else if (right)
    {
    	// Move right
    	drive_robot(0.0, -0.5);
    }
    else
    {
    	// Move forward
    	drive_robot(0.5, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
