#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "nav_msgs/msg/odometry.hpp"

class SensorListener : public rclcpp::Node
{
public:
    SensorListener() : Node("sensor_listener")
    {
        imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
            "/imu", 10,
            std::bind(&SensorListener::imu_callback, this, std::placeholders::_1));

        odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/odom", 10,
            std::bind(&SensorListener::odom_callback, this, std::placeholders::_1));
    }

private:

    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(),
        "IMU angular velocity z: %.3f",
        msg->angular_velocity.z);
    }

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(),
        "Robot position x: %.3f y: %.3f",
        msg->pose.pose.position.x,
        msg->pose.pose.position.y);
    }

    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorListener>());
    rclcpp::shutdown();
    return 0;
}
