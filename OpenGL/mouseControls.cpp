class Mouse {
public:
    static void callback(GLFWwindow* window, double xpos, double ypos) {
        Mouse* self = static_cast<Mouse*>(glfwGetWindowUserPointer(window));
        if (self) self->process(xpos, ypos);
    }

    glm::vec3 getCameraFront() const { return cameraFront; }

private:
    void process(double xpos, double ypos) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = (xpos - lastX) * sensitivity;
        float yoffset = (lastY - ypos) * sensitivity; 

        lastX = xpos;
        lastY = ypos;

        yaw   += xoffset;
        pitch += yoffset;
        pitch  = glm::clamp(pitch, -89.0f, 89.0f);

        cameraFront = glm::normalize(glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        ));
    }

    bool firstMouse = true;
    double lastX = 0.0, lastY = 0.0;
    float yaw   = -90.0f;
    float pitch =   0.0f;
    float sensitivity = 0.1f;
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
};