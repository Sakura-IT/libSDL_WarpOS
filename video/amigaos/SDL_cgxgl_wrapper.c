/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
 
#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif
#include <SDL_config.h>
/* wrapper functions for StormMesa */

#ifdef SDL_VIDEO_OPENGL

#include <string.h>
#pragma pack(2)
#include <mgl/gl.h>
#pragma pack()

/* The GL API */

/*
 * Miscellaneous
 */

 void AmiglColorTable( GLenum target, GLenum internalformat, GLint width, GLenum format, GLenum type, GLvoid *data) {
    glColorTable(target, internalformat, width, format, type, data);
 }

 void AmiglClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {
    glClearColor(red, green, blue, alpha);
 }

 void AmiglClear( GLbitfield mask ) {
    glClear(mask);
 }

 void AmiglAlphaFunc( GLenum func, GLclampf ref ) {
    glAlphaFunc(func, ref);
 }

 void AmiglBlendFunc( GLenum sfactor, GLenum dfactor ) {
    glBlendFunc(sfactor, dfactor);
 }

 void AmiglCullFace( GLenum mode ) {
    glCullFace(mode);
 }

 void AmiglFrontFace( GLenum mode ) {
    glFrontFace(mode);
 }

 void AmiglPointSize( GLfloat size ) {
    glPointSize(size);
 }

 void AmiglPolygonMode( GLenum face, GLenum mode ) {
    glPolygonMode(face, mode);
 }

 void AmiglScissor( GLint x, GLint y, GLsizei width, GLsizei height) {
    glScissor(x, y, width, height);
 }

 void AmiglDrawBuffer( GLenum mode ) {
    glDrawBuffer(mode);
 }


 void AmiglEnable( GLenum cap ) {
    glEnable(cap);
 }

 void AmiglDisable( GLenum cap ) {
    glDisable(cap);
 }

 GLboolean AmiglIsEnabled( GLenum cap ) {
    return glIsEnabled(cap);
 }

 void AmiglEnableClientState( GLenum cap ) {  /* 1.1 */
    glEnableClientState(cap);
 }

 void AmiglDisableClientState( GLenum cap ) {  /* 1.1 */
    glDisableClientState(cap);
 }

 void AmiglGetBooleanv( GLenum pname, GLboolean *params ) {
    glGetBooleanv(pname, params);
 }

 void AmiglGetFloatv( GLenum pname, GLfloat *params ) {
    glGetFloatv(pname, params);
 }

 void AmiglGetIntegerv( GLenum pname, GLint *params ) {
    glGetIntegerv(pname, params);
 }

 GLenum AmiglGetError( void ) {
    return glGetError();
 }

 const GLubyte* AmiglGetString( GLenum name ) {
    return glGetString(name);
 }

 void AmiglFinish( void ) {
    glFinish();
 }

 void AmiglFlush( void ) {
    glFlush();
 }

 void AmiglHint( GLenum target, GLenum mode ) {
    glHint(target, mode);
 }

/*
 * Depth Buffer
 */

 void AmiglClearDepth( GLclampd depth ) {
    glClearDepth(depth);
 }

 void AmiglDepthFunc( GLenum func ) {
    glDepthFunc(func);
 }

 void AmiglDepthMask( GLboolean flag ) {
    glDepthMask(flag);
 }

 void AmiglDepthRange( GLclampd near_val, GLclampd far_val ) {
    glDepthRange(near_val, far_val);
 }

/*
 * Transformation
 */

 void AmiglMatrixMode( GLenum mode ) {
    glMatrixMode(mode);
 }

 void AmiglOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
    glOrtho(left, right, bottom, top, near_val, far_val);
 }

 void AmiglFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
    glFrustum(left, right, bottom, top, near_val, far_val);
 }

 void AmiglViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
    glViewport(x, y, width, height);
 }

 void AmiglPushMatrix( void ) {
    glPushMatrix();
 }

 void AmiglPopMatrix( void ) {
    glPopMatrix();
 }

 void AmiglLoadIdentity( void ) {
    glLoadIdentity();
 }

 void AmiglLoadMatrixd( const GLdouble *m ) {
    glLoadMatrixd(m);
 }

 void AmiglLoadMatrixf( const GLfloat *m ) {
    glLoadMatrixf(m);
 }

 void AmiglMultMatrixd( const GLdouble *m ) {
    glMultMatrixd(m);
 }

 void AmiglMultMatrixf( const GLfloat *m ) {
    glMultMatrixf(m);
 }

 void AmiglRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z ) {
    glRotated(angle, x, y, z);
 }

 void AmiglRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
    glRotatef(angle, x, y, z);
 }

 void AmiglScaled( GLdouble x, GLdouble y, GLdouble z ) {
    glScaled(x, y, z);
 }

 void AmiglScalef( GLfloat x, GLfloat y, GLfloat z ) {
    glScalef(x, y, z);
 }

 void AmiglTranslated( GLdouble x, GLdouble y, GLdouble z ) {
    glTranslated(x, y, z);
 }

 void AmiglTranslatef( GLfloat x, GLfloat y, GLfloat z ) {
    glTranslatef(x, y, z);
 }

/*
 * Drawing Functions
 */

 void AmiglBegin( GLenum mode ) {
    glBegin(mode);
 }

 void AmiglEnd( void ) {
    glEnd();
 }

 void AmiglVertex2f( GLfloat x, GLfloat y )         { glVertex2f(x, y); }
 void AmiglVertex2i( GLint x, GLint y )             { glVertex2i(x, y); }

 void AmiglVertex3f( GLfloat x, GLfloat y, GLfloat z )      { glVertex3f(x, y, z); }

 void AmiglVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )       { glVertex4f(x, y, z, w); }

 void AmiglVertex2fv( GLfloat *v )    { glVertex2fv(v); }

 void AmiglVertex3fv( GLfloat *v )    { glVertex3fv(v); }

 void AmiglVertex4fv( GLfloat *v )    { glVertex4fv(v); }

 void AmiglNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )   { glNormal3f(nx, ny, nz); }

 void AmiglColor3f( GLfloat red, GLfloat green, GLfloat blue )      { glColor3f(red, green, blue); }
 void AmiglColor3ub( GLubyte red, GLubyte green, GLubyte blue ) { glColor3ub(red, green, blue); }

 void AmiglColor4f( GLfloat red, GLfloat green,
                   GLfloat blue, GLfloat alpha )    { glColor4f(red, green, blue, alpha); }
 void AmiglColor4ub( GLubyte red, GLubyte green,
                    GLubyte blue, GLubyte alpha )   { glColor4ub(red, green, blue, alpha); }

 void AmiglColor3fv( GLfloat *v ) { glColor3fv(v); }

 void AmiglColor3ubv( GLubyte *v )    { glColor3ubv(v); }

 void AmiglColor4fv( GLfloat *v ) { glColor4fv(v); }

 void AmiglColor4ubv( GLubyte *v )    { glColor4ubv(v); }

 void AmiglTexCoord2f( GLfloat s, GLfloat t )   { glTexCoord2f(s, t); }

 void AmiglTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )     { glTexCoord4f(s, t, r, q); }

 void AmiglTexCoord2fv( GLfloat *v )  { glTexCoord2fv(v); }

 void AmiglTexCoord4fv( GLfloat *v )  { glTexCoord4fv(v); }

/*
 * Vertex Arrays  (1.1)
 */

 void AmiglVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
        glVertexPointer(size, type, stride, ptr);
 }

 void AmiglColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
        glColorPointer(size, type, stride, ptr);
 }

 void AmiglTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )    {
        glTexCoordPointer(size, type, stride, ptr);
 }

 void AmiglArrayElement( GLint i )  { glArrayElement(i); }

 void AmiglDrawArrays( GLenum mode, GLint first, GLsizei count )    {
        glDrawArrays(mode, first, count);
 }

 void AmiglDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) {
        glDrawElements(mode, count, type, indices);
 }

 void AmiglInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
        glInterleavedArrays(format, stride, pointer);
 }

/*
 * Lighting
 */

 void AmiglShadeModel( GLenum mode )    { glShadeModel(mode); }


/*
 * Raster functions
 */

 void AmiglPixelStoref( GLenum pname, GLfloat param )   { glPixelStoref(pname, param); }
 void AmiglPixelStorei( GLenum pname, GLint param ) { glPixelStorei(pname, param); }

 void AmiglReadPixels( GLint x, GLint y,
                      GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      GLvoid *pixels )  {

        glReadPixels(x, y, width, height, format, type, pixels);
 }


/*
 * Texture mapping
 */

 void AmiglTexGeni( GLenum coord, GLenum pname, GLint param )       { glTexGeni(coord, pname, param); }

 void AmiglTexEnvf( GLenum target, GLenum pname, GLfloat param )    { glTexEnvf(target, pname, param); }
 void AmiglTexEnvi( GLenum target, GLenum pname, GLint param )      { glTexEnvi(target, pname, param); }

 void AmiglTexEnvfv( GLenum target, GLenum pname, GLfloat *params )   { glTexEnvfv(target, pname, params); }
 void AmiglTexEnviv( GLenum target, GLenum pname, GLint *params )     { glTexEnviv(target, pname, params); }

 void AmiglTexParameterf( GLenum target, GLenum pname, GLfloat param )  { glTexParameterf(target, pname, param); }
 void AmiglTexParameteri( GLenum target, GLenum pname, GLint param )        { glTexParameteri(target, pname, param); }

 void AmiglTexParameterfv( GLenum target, GLenum pname, const GLfloat *params ) {
        glTexParameterfv(target, pname, params);
 }
 void AmiglTexParameteriv( GLenum target, GLenum pname, const GLint *params ) {
        glTexParameteriv(target, pname, params);
 }

 void AmiglTexImage2D( GLenum target, GLint level,
                      GLint internalFormat,
                      GLsizei width, GLsizei height,
                      GLint border, GLenum format, GLenum type,
                      const GLvoid *pixels )    {

        glTexImage2D(target, level, internalFormat,
                      width, height, border, format, type, pixels);
 }


/* 1.1 functions */

 void AmiglGenTextures( GLsizei n, GLuint *textures )   {
        glGenTextures(n, textures);
 }

 void AmiglDeleteTextures( GLsizei n, const GLuint *textures)   {
        glDeleteTextures(n, textures);
 }

 void AmiglBindTexture( GLenum target, GLuint texture ) {
        glBindTexture(target, texture);
 }

 void AmiglTexSubImage2D( GLenum target, GLint level,
                     GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type,
                     const GLvoid *pixels ) {

        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
 }

/*
 * Fog
 */

 void AmiglFogf( GLenum pname, GLfloat param ) {
        glFogf(pname, param);
 }

 void AmiglFogi( GLenum pname, GLint param ) {
        glFogi(pname, param);
 }

 void AmiglFogfv( GLenum pname, GLfloat *params ) {
        glFogfv(pname, params);
 }

/* The GLU API */

/*
 *
 * Miscellaneous functions
 *
 */

 void AmigluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
                                GLdouble centerx, GLdouble centery,
                                GLdouble centerz,
                                GLdouble upx, GLdouble upy, GLdouble upz ) {

        gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
 }

 void AmigluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar ) {
        gluPerspective(fovy, aspect, zNear, zFar);
 }

/* Find the functionAddress when the name is given */
void *AmiGetGLProc(const char *proc) {
     void *func = NULL;

     if(strcmp(proc, "glClearColor") == 0) func =  AmiglClearColor;
     else if(strcmp(proc, "glClear") == 0) func =  AmiglClear;
     else if(strcmp(proc, "glAlphaFunc") == 0) func =  AmiglAlphaFunc;
     else if(strcmp(proc, "glBlendFunc") == 0) func =  AmiglBlendFunc;
     else if(strcmp(proc, "glColorTable") == 0) func =  AmiglColorTable;
     else if(strcmp(proc, "glCullFace") == 0) func =  AmiglCullFace;
     else if(strcmp(proc, "glFrontFace") == 0) func =  AmiglFrontFace;
     else if(strcmp(proc, "glPointSize") == 0) func =  AmiglPointSize;
     else if(strcmp(proc, "glPolygonMode") == 0) func =  AmiglPolygonMode;
     else if(strcmp(proc, "glScissor") == 0) func =  AmiglScissor;
     else if(strcmp(proc, "glDrawBuffer") == 0) func =  AmiglDrawBuffer;
     else if(strcmp(proc, "glEnable") == 0) func =  AmiglEnable;
     else if(strcmp(proc, "glDisable") == 0) func =  AmiglDisable;
     else if(strcmp(proc, "glIsEnabled") == 0) func =  AmiglIsEnabled;
     else if(strcmp(proc, "glEnableClientState") == 0) func =  AmiglEnableClientState;
     else if(strcmp(proc, "glDisableClientState") == 0) func =  AmiglDisableClientState;
     else if(strcmp(proc, "glGetBooleanv") == 0) func =  AmiglGetBooleanv;
     else if(strcmp(proc, "glGetFloatv") == 0) func =  AmiglGetFloatv;
     else if(strcmp(proc, "glGetIntegerv") == 0) func =  AmiglGetIntegerv;
     else if(strcmp(proc, "glGetError") == 0) func =  AmiglGetError;
     else if(strcmp(proc, "glGetString") == 0) func =  AmiglGetString;
     else if(strcmp(proc, "glFinish") == 0) func =  AmiglFinish;
     else if(strcmp(proc, "glFlush") == 0) func =  AmiglFlush;
     else if(strcmp(proc, "glHint") == 0) func =  AmiglHint;
     else if(strcmp(proc, "glClearDepth") == 0) func =  AmiglClearDepth;
     else if(strcmp(proc, "glDepthFunc") == 0) func =  AmiglDepthFunc;
     else if(strcmp(proc, "glDepthMask") == 0) func =  AmiglDepthMask;
     else if(strcmp(proc, "glDepthRange") == 0) func =  AmiglDepthRange;
     else if(strcmp(proc, "glMatrixMode") == 0) func =  AmiglMatrixMode;
     else if(strcmp(proc, "glOrtho") == 0) func =  AmiglOrtho;
     else if(strcmp(proc, "glFrustum") == 0) func =  AmiglFrustum;
     else if(strcmp(proc, "glViewport") == 0) func =  AmiglViewport;
     else if(strcmp(proc, "glPushMatrix") == 0) func =  AmiglPushMatrix;
     else if(strcmp(proc, "glPopMatrix") == 0) func =  AmiglPopMatrix;
     else if(strcmp(proc, "glLoadIdentity") == 0) func =  AmiglLoadIdentity;
     else if(strcmp(proc, "glLoadMatrixd") == 0) func =  AmiglLoadMatrixd;
     else if(strcmp(proc, "glLoadMatrixf") == 0) func =  AmiglLoadMatrixf;
     else if(strcmp(proc, "glMultMatrixd") == 0) func =  AmiglMultMatrixd;
     else if(strcmp(proc, "glMultMatrixf") == 0) func =  AmiglMultMatrixf;
     else if(strcmp(proc, "glRotated") == 0) func =  AmiglRotated;
     else if(strcmp(proc, "glRotatef") == 0) func =  AmiglRotatef;
     else if(strcmp(proc, "glScaled") == 0) func =  AmiglScaled;
     else if(strcmp(proc, "glScalef") == 0) func =  AmiglScalef;
     else if(strcmp(proc, "glTranslated") == 0) func =  AmiglTranslated;
     else if(strcmp(proc, "glTranslatef") == 0) func =  AmiglTranslatef;
     else if(strcmp(proc, "glBegin") == 0) func =  AmiglBegin;
     else if(strcmp(proc, "glEnd") == 0) func =  AmiglEnd;
     else if(strcmp(proc, "glVertex2f") == 0) func =  AmiglVertex2f;
     else if(strcmp(proc, "glVertex2i") == 0) func =  AmiglVertex2i;
     else if(strcmp(proc, "glVertex3f") == 0) func =  AmiglVertex3f;
     else if(strcmp(proc, "glVertex4f") == 0) func =  AmiglVertex4f;
     else if(strcmp(proc, "glVertex2fv") == 0) func =  AmiglVertex2fv;
     else if(strcmp(proc, "glVertex3fv") == 0) func =  AmiglVertex3fv;
     else if(strcmp(proc, "glVertex4fv") == 0) func =  AmiglVertex4fv;
     else if(strcmp(proc, "glNormal3f") == 0) func =  AmiglNormal3f;
     else if(strcmp(proc, "glColor3f") == 0) func =  AmiglColor3f;
     else if(strcmp(proc, "glColor3ub") == 0) func =  AmiglColor3ub;
     else if(strcmp(proc, "glColor4f") == 0) func =  AmiglColor4f;
     else if(strcmp(proc, "glColor4ub") == 0) func =  AmiglColor4ub;
     else if(strcmp(proc, "glColor3fv") == 0) func =  AmiglColor3fv;
     else if(strcmp(proc, "glColor3ubv") == 0) func =  AmiglColor3ubv;
     else if(strcmp(proc, "glColor4fv") == 0) func =  AmiglColor4fv;
     else if(strcmp(proc, "glColor4ubv") == 0) func =  AmiglColor4ubv;
     else if(strcmp(proc, "glTexCoord2f") == 0) func =  AmiglTexCoord2f;
     else if(strcmp(proc, "glTexCoord4f") == 0) func =  AmiglTexCoord4f;
     else if(strcmp(proc, "glTexCoord2fv") == 0) func =  AmiglTexCoord2fv;
     else if(strcmp(proc, "glTexCoord4fv") == 0) func =  AmiglTexCoord4fv;
     else if(strcmp(proc, "glVertexPointer") == 0) func =  AmiglVertexPointer;
     else if(strcmp(proc, "glColorPointer") == 0) func =  AmiglColorPointer;
     else if(strcmp(proc, "glTexCoordPointer") == 0) func =  AmiglTexCoordPointer;
     else if(strcmp(proc, "glArrayElement") == 0) func =  AmiglArrayElement;
     else if(strcmp(proc, "glDrawArrays") == 0) func =  AmiglDrawArrays;
     else if(strcmp(proc, "glDrawElements") == 0) func =  AmiglDrawElements;
     else if(strcmp(proc, "glInterleavedArrays") == 0) func =  AmiglInterleavedArrays;
     else if(strcmp(proc, "glShadeModel") == 0) func =  AmiglShadeModel;
     else if(strcmp(proc, "glPixelStoref") == 0) func =  AmiglPixelStoref;
     else if(strcmp(proc, "glPixelStorei") == 0) func =  AmiglPixelStorei;
     else if(strcmp(proc, "glReadPixels") == 0) func =  AmiglReadPixels;
     else if(strcmp(proc, "glTexGeni") == 0) func =  AmiglTexGeni;
     else if(strcmp(proc, "glTexEnvf") == 0) func =  AmiglTexEnvf;
     else if(strcmp(proc, "glTexEnvi") == 0) func =  AmiglTexEnvi;
     else if(strcmp(proc, "glTexEnvfv") == 0) func =  AmiglTexEnvfv;
     else if(strcmp(proc, "glTexEnviv") == 0) func =  AmiglTexEnviv;
     else if(strcmp(proc, "glTexParameterf") == 0) func =  AmiglTexParameterf;
     else if(strcmp(proc, "glTexParameteri") == 0) func =  AmiglTexParameteri;
     else if(strcmp(proc, "glTexParameterfv") == 0) func =  AmiglTexParameterfv;
     else if(strcmp(proc, "glTexParameteriv") == 0) func =  AmiglTexParameteriv;
     else if(strcmp(proc, "glTexImage2D") == 0) func =  AmiglTexImage2D;
     else if(strcmp(proc, "glGenTextures") == 0) func =  AmiglGenTextures;
     else if(strcmp(proc, "glDeleteTextures") == 0) func =  AmiglDeleteTextures;
     else if(strcmp(proc, "glBindTexture") == 0) func =  AmiglBindTexture;
     else if(strcmp(proc, "glTexSubImage2D") == 0) func =  AmiglTexSubImage2D;
     else if(strcmp(proc, "glFogf") == 0) func =  AmiglFogf;
     else if(strcmp(proc, "glFogi") == 0) func =  AmiglFogi;
     else if(strcmp(proc, "glFogfv") == 0) func =  AmiglFogfv;
     else if(strcmp(proc, "gluLookAt") == 0) func = AmigluLookAt;
     else if(strcmp(proc, "gluPerspective") == 0) func = AmigluPerspective;

     return func;
}

#endif /*HAVE_OPENGL*/
