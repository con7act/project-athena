//
//  TV3DManager.cpp
//  interface/src/devices
//
//  Created by Brad Hefta-Gaub on 12/24/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "InterfaceConfig.h"

#include <glm/glm.hpp>

#include <GlowEffect.h>

#include "Application.h"

#include "TV3DManager.h"
#include "Menu.h"

int TV3DManager::_screenWidth = 1;
int TV3DManager::_screenHeight = 1;
double TV3DManager::_aspect = 1.0;
eyeFrustum TV3DManager::_leftEye;
eyeFrustum TV3DManager::_rightEye;
eyeFrustum* TV3DManager::_activeEye = NULL;


bool TV3DManager::isConnected() {
    return Menu::getInstance()->isOptionChecked(MenuOption::Enable3DTVMode);
}

void TV3DManager::connect() {
    Camera& camera = *Application::getInstance()->getCamera();
    auto deviceSize = qApp->getDeviceSize();
    configureCamera(camera, deviceSize.width(), deviceSize.height());
}


// The basic strategy of this stereoscopic rendering is explained here:
//    http://www.orthostereo.com/geometryopengl.html
void TV3DManager::setFrustum(const Camera& whichCamera) {
    const double DTR = 0.0174532925; // degree to radians
    const double IOD = 0.05; //intraocular distance
    double fovy = whichCamera.getFieldOfView(); // field of view in y-axis
    double nearZ = whichCamera.getNearClip(); // near clipping plane
    double screenZ = Application::getInstance()->getViewFrustum()->getFocalLength(); // screen projection plane

    double top = nearZ * tan(DTR * fovy / 2.0); //sets top of frustum based on fovy and near clipping plane
    double right = _aspect * top; // sets right of frustum based on aspect ratio
    double frustumshift = (IOD / 2) * nearZ / screenZ;
    
    _leftEye.top = top;
    _leftEye.bottom = -top;
    _leftEye.left = -right + frustumshift;
    _leftEye.right = right + frustumshift;
    _leftEye.modelTranslation = IOD / 2;
    
    _rightEye.top = top;
    _rightEye.bottom = -top;
    _rightEye.left = -right - frustumshift;
    _rightEye.right = right - frustumshift;
    _rightEye.modelTranslation = -IOD / 2;
}

void TV3DManager::configureCamera(Camera& whichCamera_, int screenWidth, int screenHeight) {
    const Camera& whichCamera = whichCamera_;

    if (screenHeight == 0) {
        screenHeight = 1; // prevent divide by 0
    }
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _aspect= (double)_screenWidth / (double)_screenHeight;
    setFrustum(whichCamera);

    glViewport (0, 0, _screenWidth, _screenHeight); // sets drawing viewport
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TV3DManager::display(Camera& whichCamera) {
    double nearZ = whichCamera.getNearClip(); // near clipping plane
    double farZ = whichCamera.getFarClip(); // far clipping plane

    // left eye portal
    int portalX = 0;
    int portalY = 0;
    QSize deviceSize = qApp->getDeviceSize() *
        qApp->getRenderResolutionScale();
    int portalW = deviceSize.width() / 2;
    int portalH = deviceSize.height();

    ApplicationOverlay& applicationOverlay = Application::getInstance()->getApplicationOverlay();

    // We only need to render the overlays to a texture once, then we just render the texture as a quad
    // PrioVR will only work if renderOverlay is called, calibration is connected to Application::renderingOverlay() 
    applicationOverlay.renderOverlay();

    DependencyManager::get<GlowEffect>()->prepare();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    // render left side view
    glViewport(portalX, portalY, portalW, portalH);
    glScissor(portalX, portalY, portalW, portalH);
    
    Camera eyeCamera;
    eyeCamera.setRotation(whichCamera.getRotation());
    eyeCamera.setPosition(whichCamera.getPosition());

    glPushMatrix();
    {
        _activeEye = &_leftEye;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity(); // reset projection matrix
        glFrustum(_leftEye.left, _leftEye.right, _leftEye.bottom, _leftEye.top, nearZ, farZ); // set left view frustum
        GLfloat p[4][4];
        // Really?
        glGetFloatv(GL_PROJECTION_MATRIX, &(p[0][0]));
        float cotangent = p[1][1];
        GLfloat fov = atan(1.0f / cotangent);
        glTranslatef(_leftEye.modelTranslation, 0.0, 0.0); // translate to cancel parallax

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        eyeCamera.setEyeOffsetPosition(glm::vec3(-_activeEye->modelTranslation,0,0));
        Application::getInstance()->displaySide(eyeCamera, false, RenderArgs::MONO);

        applicationOverlay.displayOverlayTextureStereo(whichCamera, _aspect, fov);
        _activeEye = NULL;
    }
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);

    // render right side view
    portalX = deviceSize.width() / 2;
    glEnable(GL_SCISSOR_TEST);
    // render left side view
    glViewport(portalX, portalY, portalW, portalH);
    glScissor(portalX, portalY, portalW, portalH);
    glPushMatrix();
    {
        _activeEye = &_rightEye;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity(); // reset projection matrix
        glFrustum(_rightEye.left, _rightEye.right, _rightEye.bottom, _rightEye.top, nearZ, farZ); // set right view frustum
        GLfloat p[4][4];
        glGetFloatv(GL_PROJECTION_MATRIX, &(p[0][0]));
        GLfloat cotangent = p[1][1];
        GLfloat fov = atan(1.0f / cotangent);
        glTranslatef(_rightEye.modelTranslation, 0.0, 0.0); // translate to cancel parallax

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        eyeCamera.setEyeOffsetPosition(glm::vec3(-_activeEye->modelTranslation,0,0));
        Application::getInstance()->displaySide(eyeCamera, false, RenderArgs::MONO);

        applicationOverlay.displayOverlayTextureStereo(whichCamera, _aspect, fov);
        _activeEye = NULL;
    }
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);

    // reset the viewport to how we started
    glViewport(0, 0, deviceSize.width(), deviceSize.height());

    DependencyManager::get<GlowEffect>()->render();
}

void TV3DManager::overrideOffAxisFrustum(float& left, float& right, float& bottom, float& top, float& nearVal,
        float& farVal, glm::vec4& nearClipPlane, glm::vec4& farClipPlane) {
    if (_activeEye) {
        left = _activeEye->left;
        right = _activeEye->right;
        bottom = _activeEye->bottom;
        top = _activeEye->top;
    }
}
