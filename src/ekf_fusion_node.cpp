#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/pose2_d.hpp"

class EKFFusion : public rclcpp::Node
{
public:
    EKFFusion() : Node("ekf_fusion")
    {
        imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
            "/imu", 10,
            std::bind(&EKFFusion::imu_callback, this, std::placeholders::_1));

        odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/odom", 10,
            std::bind(&EKFFusion::odom_callback, this, std::placeholders::_1));

        pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/fused_pose", 10);

        x_ = 0.0;
        y_ = 0.0;
        theta_ = 0.0;
    }

private:

    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
    {
        double wz = msg->angular_velocity.z;

        theta_ += wz * dt_;

        RCLCPP_INFO(this->get_logger(),
        "Heading estimate (theta): %.3f",
        theta_);
    }

    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        double x_meas = msg->pose.pose.position.x;
        double y_meas = msg->pose.pose.position.y;

        // simple fusion (low pass filter)
        x_ = 0.8 * x_ + 0.2 * x_meas;
        y_ = 0.8 * y_ + 0.2 * y_meas;
        geometry_msgs::msg::PoseStamped pose_msg;

	pose_msg.header.stamp = this->now();
	pose_msg.header.frame_id = "map";

	pose_msg.pose.position.x = x_;
	pose_msg.pose.position.y = y_;
	pose_msg.pose.position.z = 0.0;

	tf2::Quaternion q;
	q.setRPY(0, 0, theta_);

	pose_msg.pose.orientation.x = q.x();
	pose_msg.pose.orientation.y = q.y();
	pose_msg.pose.orientation.z = q.z();
	pose_msg.pose.orientation.w = q.w();

	pose_pub_->publish(pose_msg);

        RCLCPP_INFO(this->get_logger(),
        "Fused pose → x: %.3f y: %.3f theta: %.3f",
        x_, y_, theta_);
    }

    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;

    double x_;
    double y_;
    double theta_;

    double dt_ = 0.1;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<EKFFusion>());
    rclcpp::shutdown();
    return 0;
}
