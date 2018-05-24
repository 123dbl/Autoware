#include "visualize_detected_objects.h"

#include <visualization_msgs/Marker.h>
#include <tf/transform_datatypes.h>

VisualizeDetectedObjects::VisualizeDetectedObjects()
{
  ros::NodeHandle private_nh_("~");
  private_nh_.param<std::string>("pointcloud_frame", pointcloud_frame_, "velodyne");

  sub_cloud_array_ = node_handle_.subscribe("/detected_objects", 1, &VisualizeDetectedObjects::callBack, this);
  pub_arrow_ = node_handle_.advertise<visualization_msgs::Marker>("/detected_objects/velocity_arrow", 1);
  pub_id_ = node_handle_.advertise<visualization_msgs::Marker>("/detected_objects/target_id", 1);
}

void VisualizeDetectedObjects::callBack(const autoware_msgs::DetectedObjectArray& input)
{
  visMarkers(input);
}

void VisualizeDetectedObjects::visMarkers(const autoware_msgs::DetectedObjectArray& input)
{
  for (size_t i = 0; i < input.objects.size(); i++)
  {
    double tv = input.objects[i].velocity.linear.x;
    double tyaw = input.objects[i].velocity.linear.y;

    visualization_msgs::Marker id;

    id.lifetime = ros::Duration(0.15);
    id.header.frame_id = pointcloud_frame_;
    id.header.stamp = input.header.stamp;
    id.ns = "id";
    id.action = visualization_msgs::Marker::ADD;
    id.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
    // green
    id.color.g = 1.0f;
    id.color.a = 1.0;
    id.id = i;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    id.pose.position.x = input.objects[i].pose.position.x;
    id.pose.position.y = input.objects[i].pose.position.y;
    id.pose.position.z = 1.5;

    // convert from RPY to quartenion
    tf::Matrix3x3 obs_mat;
    obs_mat.setEulerYPR(tyaw, 0, 0);  // yaw, pitch, roll
    tf::Quaternion q_tf;
    obs_mat.getRotation(q_tf);
    id.pose.orientation.x = q_tf.getX();
    id.pose.orientation.y = q_tf.getY();
    id.pose.orientation.z = q_tf.getZ();
    id.pose.orientation.w = q_tf.getW();

    id.scale.z = 1.0;

    std::string s_velocity = std::to_string(tv*3.6);
    std::string modified_sv = s_velocity.substr(0, s_velocity.find(".")+3);
    std::string text = "<" + std::to_string(input.objects[i].id) + "> " +
                             modified_sv + " km/h";
    // id.text = std::to_string(input.objects[i].id);
    id.text = text;

    pub_id_.publish(id);

    visualization_msgs::Marker arrow;
    arrow.lifetime = ros::Duration(0.1);

    // visualize velocity arrow only if its status is Stable
    std::string label = input.objects[i].label;
    if (label == "None" || label == "Initialized" || label == "Lost" || label == "Static")
    {
      continue;
    }

    arrow.header.frame_id = pointcloud_frame_;
    arrow.header.stamp = input.header.stamp;
    arrow.ns = "arrow";
    arrow.action = visualization_msgs::Marker::ADD;
    arrow.type = visualization_msgs::Marker::ARROW;
    // green
    arrow.color.g = 1.0f;
    arrow.color.a = 1.0;
    arrow.id = i;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    arrow.pose.position.x = input.objects[i].pose.position.x;
    arrow.pose.position.y = input.objects[i].pose.position.y;
    arrow.pose.position.z = 0.5;

    arrow.pose.orientation.x = q_tf.getX();
    arrow.pose.orientation.y = q_tf.getY();
    arrow.pose.orientation.z = q_tf.getZ();
    arrow.pose.orientation.w = q_tf.getW();

    // Set the scale of the arrow -- 1x1x1 here means 1m on a side
    // arrow.scale.x = tv;
    arrow.scale.x = 3;
    arrow.scale.y = 0.1;
    arrow.scale.z = 0.1;

    pub_arrow_.publish(arrow);
  }
}
