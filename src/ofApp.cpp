#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(0, 0, 0);
	
	ofSetLogLevel( OF_LOG_VERBOSE );
	
	wacomTablet.init(true);
	
	wacomTablet.listAttachedDevices();
	
	ofAddListener(ofEvents().touchDown, this, &ofApp::touchDown);
	ofAddListener(ofEvents().touchUp, this, &ofApp::touchUp);
	ofAddListener(ofEvents().touchMoved, this, &ofApp::touchMoved);
	
	fingersCount = 0;
	maxFingers = 16; // When you put your palms on the pad it registers odd touches
	fingers = new Finger[maxFingers];
	for( int i = 0; i < maxFingers; i++ ) {
		fingers[i].active = false;
		fingers[i].angle  = 0.0f;	
		//fingers[i].color;			
	}

}

//--------------------------------------------------------------
void ofApp::update()
{

}

//--------------------------------------------------------------
void ofApp::draw()
{
	/*
	float dimensionScale = 1.0f;
	
	for( int i = 0; i < maxFingers; i++ )
	{
		if( fingers[i].active )
		{
			// Scale the width and height up a little bit
			float tmpW = fingers[i].dimensions.x*dimensionScale*ofGetWidth();
			float tmpH = fingers[i].dimensions.y*dimensionScale*ofGetHeight();
			
			ofVec2f screenPos;
			screenPos.set(fingers[i].pos.x*ofGetWidth(), fingers[i].pos.y*ofGetHeight());
			ofSetColor(fingers[i].color);
			ofEllipse(screenPos, tmpW, tmpH );

			ofSetColor(255,255,255);	
			//fontSmall.drawString( ofToString(i), fingers[i].pos.x + tmpW*0.7, fingers[i].pos.y + 5 );
			
			//ofLogNotice() << ofToString(i) << ", " << fingers[i].pos.x << ", " << fingers[i].pos.y;

			// The angle field is always 0 on my Bamboo Touch
			/*
			ofPushMatrix();
				ofTranslate(fingers[i].pos);
				ofRotate(fingers[i].angle);
				ofLine( -10, 0, 10, 0);
			ofPopMatrix();
			 *
		}
	}
	
	ofSetColor( 255, 255, 255 );	
	*/
}

//--------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs & touch) 
{
	if( touch.id < maxFingers )	
	{
		int i = touch.id;
		fingers[i].active = true;
		fingers[i].angle  = 0.0f;	
		fingers[i].color.setHsb(ofRandom(255.0f), 255.0f, 255.0f);
	}
	fingersCount++;
}

//--------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs & touch) 
{
	//cout << "Touch MOVED " <<  touch.id << endl;
	if( touch.id < maxFingers )	
	{
		int i = touch.id;
		//ofLogNotice() << touch.x << ", " << touch.y;
		fingers[i].pos.set(touch.x, touch.y);	
		fingers[i].dimensions.set( touch.width, touch.height );
		fingers[i].angle  = touch.angle;			
	}

	if (gest.analyse(fingers, fingersCount))
		ofLogNotice() << gest.getMainMovementString();
	
}

//--------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs & touch) 
{
	//cout << "Touch UP " <<  touch.id << endl;
	if( touch.id < maxFingers )	
	{
		int i = touch.id;
		fingers[i].active = false;
	}	
	
	fingersCount--;

	if (fingersCount == 0)
		gest.reset();

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{	
}