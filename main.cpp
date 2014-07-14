//
//  main.cpp
//  smf_view
//
//  Created by 赵子扬 on 14-5-26.
//  Copyright (c) 2014年 ziyang zhao. All rights reserved.

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

#include <string.h>// Header File For The String Library
#include <iostream>
#include <fstream>
#include "Model.h"// Header File For Handling SMF Model

using namespace::std;

float xy_aspect; //Aspect of GLUI Master Window

/*** These are the live variables passed into GLUI ***/
int main_window;//mark the main window

int curr_display = 0; /*mark the display mode, 
                       0 for 'Shaded with mesh edges displayed.'
                       1 for 'Flat shaded';
                       2 for 'Smooth Shaded';
                       3 for 'Wireframe'*/

float damp_obj = 0.98;//damp factor of object rotation
float damp_view = 1.0;//damp factor of viewpoint rotation
float damp_light = 0.82;//damp factor of light position rotation

float speed_xy = 0.005; //translate speed of XY-Translation
float speed_x = 0.005;//translate speed of X-Translation
float speed_y = 0.005;//translate speed of Y-Translation
float speed_z = 0.005;//translate speed of Z-Translation

float scale = 1.0;//scale of axes

/*Text to be showed in listbox*/
char *string_list[] = { "Shaded with mesh edges displayed", "Flat shaded", "Smooth Shaded", "Wireframe" };

GLfloat light0_ambient[] = {0.1f, 0.1f, 0.3f, 1.0f};//light0 ambient
GLfloat light0_diffuse[] = {0.6f, 0.6f, 1.0f, 1.0f};//light0 diffuse
GLfloat light0_position[] = {2.0f, 2.0f, 5.5f, 0.0f};//light0 position

float object_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };//object rotation
float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };//viewpoint rotation
GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };//light rotation
float obj_pos[] = { 0.0, 0.0, 0.0 };//object position

bool isUseDefaultModel = true;/*use default model or imported smf model
                               'true' for use default model
                               'false' for use imported smf model*/

/*** Pointers to the windows and some of the controls we'll create ***/
GLUI *glui;//glui window
GLUI_Translation * trans_xy, * trans_x, * trans_y, * trans_z;//glui translation button
GLUI_Rotation *obj_rot, *view_rot, *lights_rot;//glui rotation button

/***Pointer to the SMF model we imported***/
Model *model = NULL;

/********** User IDs for callbacks ********/
#define QUIT_ID             0

#define OPEN_ID         101
#define SAVE_ID         102

#define SPEED_XY_ID         201
#define SPEED_X_ID          202
#define SPEED_Y_ID          203
#define SPEED_Z_ID          204

#define USE_DEFAULT_MODEL 301


/**************************************** setToDefault() *******************/
void setToDefault()
{
    /*I do not reset the light and rotations here*/
    
    obj_rot->reset();
    lights_rot->reset();
    view_rot->reset();
    obj_pos[0] = obj_pos[1] = obj_pos[2] = 0;
    
    curr_display = 0;
    
    damp_obj = 0.98;
    damp_view = 1.0;
    damp_light = 0.82;
    
    speed_xy = 0.005;
    speed_x = 0.005;
    speed_y = 0.005;
    speed_z = 0.005;
    
    scale = 1.0;
    
    /*clear the last model*/
    if (model) {
        delete model;
        model = NULL;
    }
}

/**************************************** open_cb() *******************/
void open_cb()
{
    string open_path;//the path of the smf file to be open
    ifstream file;//the smf file
    string format;//format of the file
    
    /*continuely ask the user to input a smf file path, until the path is usable*/
    while (true) {
        cout<<"Please enter the path of a SMF file:"<<endl;
        getline(cin,open_path);//get the path
        
        /*check if the path ends with '.smf' */
        format = open_path.substr(open_path.length()-4, 4);
        if (0 != strcmp(format.c_str(), ".smf")) {
            cout<<"This file is not a SMF file! "<<endl;
            continue;
        }
        
        /*if this file is a smf file, try to open the file*/
        file.open(open_path.c_str());
        if (!file.is_open()) {
            cout<<"file does not exist!"<<endl;
            continue;
        }
        
        /*this file is successfully opened*/
        else {
            cout<<"Open successfully!"<<endl;
            break;
        }
    }
    
    /*Since the user import an model, set this mark to false*/
    isUseDefaultModel = false;
    
    /*Set everything  to default*/
    setToDefault();
    
    /*Read file*/
    string flag;//the flag is the fisrt character in each line, usually is '#', 'v', 'f'
    int vertexNum, faceNum;//read from the first line of the file
    
    GLfloat x, y, z;//coordinates of the vertexs
    /*store the coordinate ranges of this model*/
    GLfloat xMin = 10000, xMax = -10000, yMin = 10000, yMax = -10000, zMin = 10000, zMax = -10000;
    int v1, v2, v3;//the indexes of the three vertexs of each triangle face
    
    int vCount = 0;//current vertex count
    int fCount = 0;//current face count
    
    /*read the first line from the file, store the number of vertex and face*/
    file>>flag>>vertexNum >>faceNum;
    
    /*this pointer has been set to null, build the new model*/
    model = new Model(vertexNum, faceNum);
    
    /*read from the second line to the end of the file*/
    while (file>>flag) {
        if (!strcmp(flag.c_str(), "v")) {/*If this line begins with a 'v', add a vertex to the model*/
            /*read the coordinates of a vertex, store it in the model*/
            file>>x>>y>>z;
            model->AddVertex(vCount, x, y, z);
            
            /*calculate the range of this model*/
            if (x >xMax) {
                xMax = x;
            }else if (x < xMin) {
                xMin = x;
            }
            if (y >yMax) {
                yMax = y;
            }else if (y < yMin) {
                yMin = y;
            }
            if (z >zMax) {
                zMax = z;
            }else if (z < zMin) {
                zMin = z;
            }
            
            vCount++;
            continue;
        }else if (!strcmp(flag.c_str(), "f")){/*If this line begins with a 'f', add a face to the model*/
            /*read the indexes of  the  vertexs of a face, store it in the model*/
            file>>v1>>v2>>v3;
            model->AddFace(fCount, v1, v2, v3);
            fCount++;
            continue;
        }else {/*If this line begins something else, ignore this line*/
            continue;
        }
    }
    
    /*store the range of the model*/
    model->AddPosition(xMin, xMax, yMin, yMax, zMin, zMax);
    
    /*close the file*/
    file.close();

    /*Redisplay*/
    glutPostRedisplay();
}


/**************************************** save_cb() *******************/
void save_cb()
{
    /*if model does not exist, we are using the default model*/
    if (model == NULL) {
        cout<<"Sorry, default model can not be saved!"<<endl;
    }
    else
    {/*use the function in Model.h to save the model*/
        model->SaveModel();
    }
}


/**************************************** control_cb() *******************/
/* GLUI control callback                                                 */

void control_cb( int control )
{
    switch (control) {
        case QUIT_ID://user pressed the 'quit' button
            exit(0);//quit
            break;
        case OPEN_ID://user pressed the 'open' button
            open_cb();
            break;
        case SAVE_ID://user pressed the 'save' button
            save_cb();
            break;
        case SPEED_XY_ID://user changed the XY-translation speed(using the spinner)
            trans_xy->set_speed(speed_xy);
            break;
        case SPEED_X_ID://user changed the X-translation speed(using the spinner)
            trans_x->set_speed(speed_x);
            break;
        case SPEED_Y_ID://user changed the Y-translation speed(using the spinner)
            trans_y->set_speed(speed_y);
            break;
        case SPEED_Z_ID://user changed the Z-translation speed(using the spinner)
            trans_z->set_speed(speed_z);
            break;
        case USE_DEFAULT_MODEL://user pressed the 'UseDefaultModel' button
        {
            isUseDefaultModel = true;
            setToDefault();//set everything to default
            glutPostRedisplay();//Redisplay
        }
        default:
            break;
    };
}


/**************************************** myGlutKeyboard() **********/

void myGlutKeyboard(unsigned char Key, int x, int y)
{
    switch (Key) {
        /*If the user press the Key 'q', then quit the program*/
        case 27:
        case 'q':
            exit(0);
            break;
        default:
            break;
    };
}

/***************************************** myGlutMenu() ***********/

void myGlutMenu( int value )
{
    myGlutKeyboard( value, 0, 0 );
}


/***************************************** myGlutIdle() ***********/

void myGlutIdle( void )
{
/*    if (glutGetWindow() != main_window) {
        glutSetWindow(main_window);
    }*/
    
    glutPostRedisplay();
}

/***************************************** myGlutMouse() **********/

void myGlutMouse(int button, int button_state, int x, int y )
{
}

/***************************************** myGlutMotion() **********/

void myGlutMotion(int x, int y )
{

    glutPostRedisplay();
}

/**************************************** myGlutReshape() *************/

void myGlutReshape( int x, int y ){
    int tx, ty, tw, th;
    
    /*get viewpoint*/
    GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
    glViewport(tx, ty, tw, th);
    
    xy_aspect = (float )tw / (float)th;
    
    glutPostRedisplay();
}

/************************************************** draw_axes() **********/
/* Disables lighting, then draws RGB axes                                */

void draw_axes( float scale )
{
    glDisable( GL_LIGHTING );
    
    glPushMatrix();
    glScalef( scale, scale, scale );
    
    glBegin( GL_LINES );//draw lines
    
    glColor3f( 1.0, 0.0, 0.0 );
    glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 ); /* X axis, red color*/
    
    glColor3f( 0.0, 1.0, 0.0 );
    glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 ); /* Y axis, green color*/
    
    glColor3f( 0.0, 0.0, 1.0 );
    glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 ); /* Z axis, blue color*/
    glEnd();
    
    glPopMatrix();
    
    glEnable( GL_LIGHTING );
}

/***************************************** drawObject() ********************/
void drawObject()
{
    /*if use the default model*/
    if (true == isUseDefaultModel) {
        glTranslatef(0.0, 0.0, 0.0);
        glutSolidTeapot(0.5f);
        return;
    }else {/*if use the imported model*/
        if (model == NULL) {//this usually does not happen
            return;
        }
        bool isFlat = false;
        if (curr_display == 1) {/*if is flat shaded, use vertex normal to draw the model
                                 else, use vertex normal to draw the model*/
            isFlat = true;
        }
        model->DrawModel(isFlat);
    }
}


/***************************************** myGlutDisplay() *****************/
void myGlutDisplay( void )
{
    /*clear color and depth*/
    glClearColor( .9f, .9f, .9f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-xy_aspect*0.04, xy_aspect*0.04, -0.04, 0.04, 0.1, 15.0);
    
    
    glMatrixMode( GL_MODELVIEW );
    
    /*set light*/
    glLoadIdentity();
    glMultMatrixf(lights_rotation);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    /*set viewpoint*/
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -2.6f );
    glTranslatef(obj_pos[0], obj_pos[1], -obj_pos[2]);
    glMultMatrixf(view_rotate);
    
    draw_axes(1);//draw axes
    
    /*** Now we render object, using the variables 'segments' and
     'wireframe'.  These are _live_ variables, which are transparently
     updated by GLUI ***/
    
    /*draw object*/
    glPushMatrix();
    glMultMatrixf(object_rotate);
    
    if (0== curr_display){//shaded with mesh edges displayed
        glPolygonOffset(1, 1);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glShadeModel(GL_SMOOTH);//shade mode is smooth
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );//draw faces
        drawObject();
        glPopMatrix();
        
        /*draw the model again, use wireframe*/
        glPushMatrix();
        glMultMatrixf( object_rotate );
        /*use wireframe as material*/
        glColorMaterial(GL_FRONT, GL_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(0.0f, 0.0f, 0.0f);//set the color of lines to blace
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);//draw lines
        drawObject();
        glColor3f(0.9f, 0.9f, 0.9f);//set the color back
        glDisable(GL_COLOR_MATERIAL);//disable material
    }
   else if (1 == curr_display) {//Flat shaded
        glShadeModel(GL_FLAT);//shade mode is flat
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );//draw faces
        drawObject();
    }
    else if (2 == curr_display){//smooth shaded
        glShadeModel(GL_SMOOTH);//shade model is smooth
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );//draw faces
        drawObject();
    }
    else if (3 == curr_display){//wireframe
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );//whatever shade mode is , draw lines
        drawObject();
    }

    glPopMatrix();
    
    glEnable(GL_LIGHTING);
    
    glutSwapBuffers();
}

/**************************************** main() ********************/

int main(int argc, char* argv[])
{
    /****************************************/
    /*   Initialize GLUT and create window  */
    /****************************************/
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(900, 750);
    
    main_window = glutCreateWindow("Ziyang's GUI for HW1");
    glutDisplayFunc(myGlutDisplay);
    GLUI_Master.set_glutReshapeFunc(myGlutReshape);
    GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);
    GLUI_Master.set_glutSpecialFunc(NULL);
    GLUI_Master.set_glutMouseFunc(myGlutMouse);
    glutMotionFunc(myGlutMotion);
    
    /****************************************/
    /*       Set up OpenGL lights           */
    /****************************************/
    
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    
    /****************************************/
    /*          Enable z-buferring          */
    /****************************************/
    
    glEnable(GL_DEPTH_TEST);
    
    /****************************************/
    /*         Here's the GLUI code         */
    /****************************************/
    
    
    /*** Create the side subwindow ***/
    glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
    
    
    /*** Create a panel for the display options ***/
    GLUI_Panel *panel1 = new GLUI_Panel(glui, "Display Options", GLUI_PANEL_NONE);
    
    GLUI_Listbox * listbox = new GLUI_Listbox(panel1, "Display:", &curr_display);//create listbox for display options
    for (int i = 0; i < 4; i++) {//add four options
        listbox->add_item(i, string_list[i]);
    }
    
    new GLUI_StaticText(panel1, "");
    
    new GLUI_Button(panel1, "Use Default Model", USE_DEFAULT_MODEL, control_cb);//'Use Default Model' button
    new GLUI_StaticText(glui, "");
    
    
    /*** Create a panel for rotation options ***/
    GLUI_Panel *panel2 = new GLUI_Panel(glui, "Rotation Options", GLUI_PANEL_NONE);
    
    /*Rotate objects*/
    obj_rot = new GLUI_Rotation(panel2, "Objects", object_rotate);
    obj_rot->set_spin(damp_obj);
    
    new GLUI_Column(panel2, true);
    
    /*Rotate viewpoint*/
    view_rot = new GLUI_Rotation(panel2, "Viewpoint", view_rotate);
    view_rot->set_spin(damp_view);
    
    new GLUI_Column(panel2, true);
    
    /*Rotate light*/
    lights_rot = new GLUI_Rotation(panel2, "Light", lights_rotation);
    lights_rot->set_spin(damp_light);
    
    new GLUI_StaticText(glui, "");
    
    /*** Create a panel for translation options ***/
    GLUI_Panel *panel3 = new GLUI_Panel(glui, "Translation Options");
    
    /*xy translation*/
    trans_xy = new GLUI_Translation(panel3, "Objects XY", GLUI_TRANSLATION_XY, obj_pos);
    trans_xy->set_speed(speed_xy);
    GLUI_Spinner *spinner_xy = new GLUI_Spinner(panel3, "Speed:", &speed_xy, SPEED_XY_ID, control_cb);
    spinner_xy->set_float_limits(0.001, 0.01);
    
    new GLUI_StaticText(panel3, "");
    
    /*x translation*/
    trans_x = new GLUI_Translation(panel3, "Object X", GLUI_TRANSLATION_X, obj_pos);
    trans_x->set_speed(speed_x);
    GLUI_Spinner *spinner_x = new GLUI_Spinner(panel3, "Speed:", &speed_x, SPEED_X_ID, control_cb);
    spinner_x->set_float_limits(0.001, 0.01);
    
    new GLUI_Column(panel3, true);
    
    /*y translation*/
    trans_y = new GLUI_Translation(panel3, "Object Y", GLUI_TRANSLATION_Y, &obj_pos[1] );
    trans_y->set_speed(speed_y);
    GLUI_Spinner *spinner_y = new GLUI_Spinner(panel3, "Speed:", &speed_y, SPEED_Y_ID, control_cb);
    spinner_y->set_float_limits(0.001, 0.01);
    
    new GLUI_StaticText(panel3, "");
    
    /*z translation*/
    trans_z = new GLUI_Translation(panel3, "Object Z", GLUI_TRANSLATION_Z, &obj_pos[2]);
    trans_z->set_speed(0.005);
    GLUI_Spinner *spinner_z = new GLUI_Spinner(panel3, "Speed:", &speed_z, SPEED_Z_ID, control_cb);
    spinner_z->set_float_limits(0.001, 0.01);
    
    new GLUI_StaticText(glui, "");
    
    /*** Create a panel for buttons ***/
    GLUI_Panel *panel4 = new GLUI_Panel(glui, "", GLUI_PANEL_NONE);
    new GLUI_Button(panel4, "Open", OPEN_ID, control_cb);//"open" button
    
    new GLUI_Column(panel4, false);
    new GLUI_Button(panel4, "Save", SAVE_ID, control_cb);//"save" button
    
    new GLUI_StaticText(glui, "");
    new GLUI_Button(glui, "Quit", QUIT_ID, (GLUI_Update_CB)exit);//"quit" button
    //text = new GLUI_EditText(glui, "Text", s, TEXT_ID, control_cb);
    
    glui->set_main_gfx_window(main_window);
    
    glutMainLoop();
    return EXIT_SUCCESS;
}

