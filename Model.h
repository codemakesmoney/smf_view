//
//  Model.h
//  smf_view
//
//  Created by 赵子扬 on 14-6-1.
//  Copyright (c) 2014年 ziyang zhao. All rights reserved.
//

#ifndef __smf_view__Model__
#define __smf_view__Model__

#include <iostream>
#include <fstream>
#include <math.h>

#if defined(GLUI_FREEGLUT)

// FreeGLUT does not yet work perfectly with GLUI
//  - use at your own risk.

#include <GL/freeglut.h>

#elif defined(GLUI_OPENGLUT)

// OpenGLUT does not yet work properly with GLUI
//  - use at your own risk.

#include <GL/openglut.h>

#else

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <GLUI/glui.h>// Header File For The GLUI Library
#include <OpenGL/gl.h> // Header File For The OpenGL Library
#include <OpenGL/glu.h> // Header File For The GLu Library
#else
#include <GL/glut.h>
#include <GL/glui.h>// Header File For The GLUI Library
#include <GL/gl.h> // Header File For The OpenGL Library
#include <GL/glu.h> // Header File For The GLu Library
#endif

#endif

using namespace std;


class Model
{
public:
    Model(int, int);//build model use vertex and face number
    ~Model();
    
    void AddVertex(int, GLfloat, GLfloat, GLfloat);//add a vertex to the model
    void AddFace(int, int, int, int);//add a face to the model
    /*User the max/min x/y/z coordinate to compute the model's middle point and range*/
    void AddPosition(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
    
    void DrawModel(bool);//draw the model
    void SaveModel();//save the model to a smf file
    Model* OpenModel();//nothing there
    
private:
    GLfloat **vertexArray;//the 2d array that store vertex information
    int **faceArray;//the 2d array that store face information
    GLfloat **vertexNormalArray;//the 2d array that store the normal of every vertex
    GLfloat **faceNormalArray;//the 2d array that store the normal of every face
    int vertexNum;//number of vertex
    int faceNum;//number of face
    GLfloat xMiddle, yMiddle, zMiddle;//middle point's coordinate
    GLfloat range;//model maximum range
    
    void computeVertexNormalUnit();
};

#endif /* defined(__smf_view__Model__) */
