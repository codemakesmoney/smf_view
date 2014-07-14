//
//  Model.cpp
//  smf_view
//
//  Created by 赵子扬 on 14-6-1.
//  Copyright (c) 2014年 ziyang zhao. All rights reserved.
//

#include "Model.h"

/********************ComputeNormal()***********************/
/*Compute the normal vecter of a triangle face*/
void computeFaceNormal(GLfloat *v1, GLfloat*v2, GLfloat* v3, GLfloat* normal)
{
    GLfloat temp1[3];
    GLfloat temp2[3];
    for (int i = 0; i < 3; i++) {
        temp1[i] = v1[i] -v2[i];
        temp2[i] = v2[i] - v3[i];
    }
    
    normal[0] = temp1[1] * temp2 [2] - temp1[2] * temp2[1];
    normal[1] = temp1[0] * temp2 [2] - temp1[2] * temp2[0];
    normal[2]=  temp1[0] * temp2 [1] - temp1[1] * temp2[0];
    
    
    /*convert this normal vector to a unit vector*/
    GLfloat length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (length == 0) {
        length = 1.0f;
    }
    for (int i = 0; i < 3; i++) {
        normal[i] /= length;
    }
    return;
}

/********************computeVertexNormalUnit()***********************/
/*Compute the unit normal vecter of all the vertexes*/
void Model::computeVertexNormalUnit()
{
    GLfloat length;
    for (int i = 0; i < vertexNum; i++) {
        length = sqrt(vertexNormalArray[i][0] * vertexNormalArray[i][0]
                      + vertexNormalArray[i][1] * vertexNormalArray[i][1]
                      + vertexNormalArray[i][2] * vertexNormalArray[i][2]);
        if (length == 0) {
            length = 1.0f;
        }
        for (int j = 0; j < 3; j++) {
            vertexNormalArray[i][j] /= length;
        }
    }
    return;
}

/****************Model(int, int)************************/
/***use the number of vertex and face to build model************/
Model::Model(int vn, int fn)
{
    vertexNum = vn;
    faceNum = fn;
    
    /*inialize vertex and vertex normal*/
    vertexArray = new GLfloat*[vertexNum];
    vertexNormalArray = new GLfloat*[vertexNum];
    for(int i = 0; i < vertexNum; i++)
    {
        vertexArray[i] = new GLfloat[3];
        vertexNormalArray[i] = new GLfloat[3];
    }
    
    /*inialize face and face normal*/
    faceArray = new int*[faceNum];
    faceNormalArray = new GLfloat*[faceNum];
    for(int i = 0; i < faceNum; i++)
    {
        faceArray[i] = new int[3];
        faceNormalArray[i] = new GLfloat[3];
    }
}

/****************~Model()************************/
Model::~Model()
{
    delete vertexArray;
    delete faceArray;
    delete vertexNormalArray;
    delete faceNormalArray;
}

/****************void AddVertex()************************/
void Model::AddVertex(int order, GLfloat x, GLfloat y, GLfloat z)
{
    /*if the vertex order is right, add the coordinate to 2d array, if not, return*/
    if (order < vertexNum && order >= 0) {
        GLfloat *vcurr= vertexArray[order];
        vcurr[0] = x;
        vcurr[1] = y;
        vcurr[2] = z;
    }
}

/****************void AddFace()************************/
void Model::AddFace(int order, int v1, int v2, int v3)
{
    /*if the face order is right, add the index of vertex to 2d array, if not, return*/
    if (order < faceNum && order >= 0) {
        int *fcurr = faceArray[order];
        fcurr[0] = v1;
        fcurr[1] = v2;
        fcurr[2] = v3;
        
        //GLfloat normal[3];

        //compute the normal of this face
        computeFaceNormal(vertexArray[v1 - 1], vertexArray[v2 - 1], vertexArray[v3 - 1], faceNormalArray[order]);
        
        //compute the normal of this vertexes
        for (int i = 0; i < 3; i++) {
            //faceNormalArray[order][i] = normal[i];
            vertexNormalArray[v1 - 1][i] += faceNormalArray[order][i];
            vertexNormalArray[v2 - 1][i] += faceNormalArray[order][i];
            vertexNormalArray[v3 - 1][i] += faceNormalArray[order][i];
        }
    }
   
    //if this is the last face, compute the normal of this vertexs
    if (order == faceNum - 1) {
        computeVertexNormalUnit();
    }
}


/****************void AddPosition()************************/
void Model::AddPosition(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax, GLfloat zMin, GLfloat zMax)
{
    /*compute the middle point of the model*/
    xMiddle = (xMin + xMax) / 2;
    yMiddle = (yMin + yMax) / 2;
    zMiddle = (zMin + zMax) / 2;
    
    /*compute the range of the model: find the maximum among  x-range, y-range and z-range*/
    range = xMax - xMin;
    if ((xMax - xMin) < (yMax - yMin)){
        range = yMax - yMin;
    }
    if (range < (zMax - zMin)) {
        range = zMax - zMin;
    }
}

/****************void DrawModel()************************/
void Model::DrawModel(bool isFlat)
{
    GLfloat *v1, *v2, *v3;
    GLfloat scale = 1 / range;
    
    /*put the middle point of the model at the middle of the screen*/
    glTranslatef(-xMiddle, -yMiddle, -zMiddle);
    
    /*adjust the model to the best scale*/
    glScalef( scale, scale, scale );
    glEnable(GL_NORMALIZE);
    
    GLfloat *normal;
    glBegin(GL_TRIANGLES);//draw triangles
    int *face = NULL;
    
    /*for each face, get its vertexs' index, use the indexes to get the vertex's coordinate, connect the vertex*/
    for (int i = 0; i < faceNum; i++) {
        face = faceArray[i];
        v1 = vertexArray[face[0] -1];
        v2 = vertexArray[face[1] -1];
        v3 = vertexArray[face[2] -1];
        
        if (isFlat) {/*if is flat, use face normal to draw this model*/
            normal = faceNormalArray[i];//compute the normal of this triangle
            glNormal3fv(normal);
            glVertex3fv(v1);
            glVertex3fv(v2);
            glVertex3fv(v3);
        }
        else{/*if not flat, use vertex normal to draw this model*/
            glNormal3fv(vertexNormalArray[face[0] - 1]);//get the normal of each vertex
            glVertex3fv(v1);
            glNormal3fv(vertexNormalArray[face[1] - 1]);
            glVertex3fv(v2);
            glNormal3fv(vertexNormalArray[face[2] - 1]);
            glVertex3fv(v3);
        }
    }
    
/*    if (withNormal) {
        glDisable(GL_NORMALIZE);
    }*/
    glScalef( range, range, range );
    glEnd();
}

/****************void SaveModel()************************/
void Model::SaveModel()
{
    string save_path;//the path to save the file
    string format;//format of the file
    ofstream file;//the file to save
    
    /*ask to user to input a usable path*/
    while (true)
    {
        cout<<"Please enter the path to save:"<<endl;
        getline(cin, save_path);
        
        /*if the path does not ends with '.smf', ask the user to input again*/
        format = save_path.substr(save_path.length()-4, 4);
        if (0 != strcmp(format.c_str(), ".smf"))
        {
            cout<<"This path does not end with '.smf', please enter again"<<endl;
            continue;
        }//end if
        
        file.open(save_path.c_str());
        if (file.is_open())
        {
            break;
        }//end outer else
        else//if the file does not exist, ask the user to input another path
        {
            cout<<"Failed to open the path!"<<endl;
            continue;
        }//end outer else
    }//end while
    
    GLfloat*vertex = NULL;// a point to a vertex
    int *face = NULL;// a point to a face
    
    /*write the first line to the file*/
    file<<"# "<<vertexNum<<" "<<faceNum<<"\n";
    
    /*write every vertex to the file*/
    for (int i = 0; i < vertexNum; i++) {
        vertex = vertexArray[i];
        file<<"v "<<vertex[0]<<" "<<vertex[1]<<" "<<vertex[2]<<"\n";
    }
    
    /*write every face to the file*/
    for (int i = 0; i < faceNum; i++) {
        face = faceArray[i];
        file<<"f "<<face[0]<<" "<<face[1]<<" "<<face[2]<<"\n";
    }
    
    cout<<"successfully saved!"<<endl;
    file.close();
}

Model* Model::OpenModel()
{
    return NULL;
}

