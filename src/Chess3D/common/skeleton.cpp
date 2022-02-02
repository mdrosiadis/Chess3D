#include "skeleton.h"
#include "model.h"
#include <glm/gtc/matrix_transform.hpp>

void Joint::updateWorldTransformation() {
    if (parent == NULL) // root
    {
        jointWorldTransformation = jointLocalTransformation;
    } else {
        jointWorldTransformation =
            parent->jointWorldTransformation * jointLocalTransformation;
    }
}

Body::~Body() {
    for (Drawable* d : drawables) {
        delete d;
    }
}

void Body::draw(
    const GLuint& modelMatrixLocation,
    const GLuint& viewMatrixLocation,
    const GLuint& projectionMatrixLocation,
    const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix) {
    joint->updateWorldTransformation();
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                       &joint->jointWorldTransformation[0][0]);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE,
                       &projectionMatrix[0][0]);

    for (Drawable* d : drawables) {
        d->bind();
        d->draw();
    }
}

Skeleton::Skeleton(
    GLuint modelMatrixLocation,
    GLuint viewMatrixLocation,
    GLuint projectionMatrixLocation) :
    modelMatrixLocation(modelMatrixLocation),
    viewMatrixLocation(viewMatrixLocation),
    projectionMatrixLocation(projectionMatrixLocation) {
}

Skeleton::~Skeleton() {
    for (auto body : bodies) {
        delete body.second;
    }

    for (auto joint : joints) {
        delete joint.second;
    }
}

void Skeleton::setPose(const std::map<int, glm::mat4>& jointTransformations) {
    for (const auto& tran : jointTransformations) {
        joints[tran.first]->jointLocalTransformation = tran.second;
    }
}

void Skeleton::draw(const glm::mat4 & viewMatrix, const glm::mat4 & projectionMatrix) {
    for (auto& body : bodies) {
        body.second->draw(modelMatrixLocation, viewMatrixLocation,
                          projectionMatrixLocation, viewMatrix, projectionMatrix);
    }
}

std::map<int, glm::mat4> Skeleton::getJointWorldTransformations() {
    std::map<int, glm::mat4> jointWorldTransformations;
    // update before computing
    for (auto joint : joints) {
        joint.second->updateWorldTransformation();
    }

    for (auto joint : joints) {
        jointWorldTransformations[joint.first] = joint.second->jointWorldTransformation;
    }

    return  jointWorldTransformations;
}