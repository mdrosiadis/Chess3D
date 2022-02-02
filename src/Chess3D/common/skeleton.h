#ifndef SKELETON_H
#define SKELETON_H

#include <GL/glew.h>
#include <vector>
#include <map>
#include <glm/glm.hpp>

class Drawable;

struct Joint {
    Joint* parent = NULL;
    glm::mat4 jointLocalTransformation, jointWorldTransformation,
        jointBindTransformation;

    /** After updating the jointLocalTransformation call this method to compute
    * the jointWorldTransformation
    */
    void updateWorldTransformation();
};

struct Body {
    Joint* joint;  // not owned by the body, just a reference pointer
    std::vector<Drawable*> drawables; // owned by the body, thus must be freed

    /* Free all drawables (a body can have many drawables)*/
    ~Body();

    /* Given the view and projection matrix draw every attached drawables */
    void draw(
        const GLuint& modelMatrixLocation,
        const GLuint& viewMatrixLocation,
        const GLuint& projectionMatrixLocation,
        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
};

struct Skeleton {
    std::map<int, Body*> bodies;
    std::map<int, Joint*> joints;

    // shader locations to M, V, P
    GLuint modelMatrixLocation, viewMatrixLocation, projectionMatrixLocation;

    Skeleton(
        GLuint modelMatrixLocation,
        GLuint viewMatrixLocation,
        GLuint projectionMatrixLocation);

    /* Free all bodies and joints*/
    ~Skeleton();

    /* Update joint local coordinates */
    void setPose(const std::map<int, glm::mat4>& jointTransformations);

    /* Given the view and projection matrix draw every attached drawables */
    void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    /* Get joint world transformations after setting the pose */
    std::map<int, glm::mat4> getJointWorldTransformations();
};

#endif