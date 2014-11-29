#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/init.h>
#include <wx/app.h>
#include <wx/font.h>
#include "simulatorInterface.h"
#include <math.h>
#include <stdio.h>
#include <vector>

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
wxCoord stepSize = 40;


// Define the static member variables
wxCoord SimulatorInterface::ROBOT_X = 9;
wxCoord SimulatorInterface::ROBOT_Y = 6;
wxCoord SimulatorInterface::GOAL_X = 15;
wxCoord SimulatorInterface::GOAL_Y = 6;

/*
*	Creates a simulator with a vector of actions
*/
Simulator::Simulator(std::vector<int> *list) :
actions(list)
{	
}

/*
*	Creates a simulator interface with a vector of actions
*/
SimulatorInterface::SimulatorInterface(std::vector<int> *list)
{
	actions = list;
	currentAction = 0;
	SimulatorInterface::ROBOT_X = 9;
	SimulatorInterface::ROBOT_Y = 6;
	SimulatorInterface::GOAL_X = 15;
	SimulatorInterface::GOAL_Y = 6;
}

/*
*	Runs the simulation without displaying it
*/
void SimulatorInterface::runSimulation() {	
	while(currentAction < actions->size()) {
		switch((*actions)[currentAction]) {
			case 0:		// If the action is 0, move right
				SimulatorInterface::ROBOT_X += 1;
				break;
			case 2:		// If the action is 2, move left
				SimulatorInterface::ROBOT_X -= 1;
				break;
			case 1:		// If the action is 1, move up
				SimulatorInterface::ROBOT_Y -= 1;
				break;
			case 3:		// If the action is 3, move down
				SimulatorInterface::ROBOT_Y += 1;
				break;
		}
		currentAction++;
	} 
}


/*
*	Opens a GUI that displays the simulation
*/
void SimulatorInterface::displaySimulation() {
	int argc = 0;
	char ** argv = 0;

	wxApp* simulator = new Simulator(actions);
	wxApp::SetInstance(simulator);	
	wxEntry(argc, argv);
	wxEntryCleanup();
}


float SimulatorInterface::getFitnessValue() {
	wxCoord x = SimulatorInterface::GOAL_X - SimulatorInterface::ROBOT_X;
	wxCoord y = SimulatorInterface::GOAL_Y - SimulatorInterface::ROBOT_Y;
	float distance = sqrt(x*x + y*y);	
	if(distance == 0.0)
		return 10000.0;	
	return 1.0/distance;
}
 
 
bool Simulator::OnInit()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Robot Simulator"), wxPoint(50,50), wxSize(WINDOW_WIDTH,WINDOW_HEIGHT));
 
	drawPane = new BasicDrawPane( (wxFrame*) frame, actions);
	sizer->Add(drawPane, 1, wxEXPAND);
 
	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);
 
	frame->Show();
    return true;
} 
 
BEGIN_EVENT_TABLE(BasicDrawPane, wxPanel)
// some useful events
/*
 EVT_MOTION(BasicDrawPane::mouseMoved)
 EVT_LEFT_UP(BasicDrawPane::mouseReleased)
 EVT_LEAVE_WINDOW(BasicDrawPane::mouseLeftWindow)
 EVT_KEY_UP(BasicDrawPane::keyReleased)
 EVT_MOUSEWHEEL(BasicDrawPane::mouseWheelMoved)
 */
 
// catch paint events
EVT_LEFT_DOWN(BasicDrawPane::leftClick)
EVT_RIGHT_DOWN(BasicDrawPane::rightClick)
EVT_KEY_DOWN(BasicDrawPane::keyPressed)
EVT_PAINT(BasicDrawPane::paintEvent)
 
END_EVENT_TABLE()
 
 
// some useful events
/*
 void BasicDrawPane::mouseMoved(wxMouseEvent& event) {}
 void BasicDrawPane::mouseDown(wxMouseEvent& event) {}
 void BasicDrawPane::mouseWheelMoved(wxMouseEvent& event) {}
 void BasicDrawPane::mouseReleased(wxMouseEvent& event) {}
 void BasicDrawPane::rightClick(wxMouseEvent& event) {}
 void BasicDrawPane::mouseLeftWindow(wxMouseEvent& event) {}
 void BasicDrawPane::keyReleased(wxKeyEvent& event) {}
 */
 
BasicDrawPane::BasicDrawPane(wxFrame* parent, std::vector<int> *list) :
wxPanel(parent), actions(list), currentAction(0)
{
}

/*
 * When a left clicked is detected execute the next action in the simulation
 *
 */
void BasicDrawPane::leftClick(wxMouseEvent& event)
{
	step();

	// Redraw the window
	Refresh();
}


/*
 * When a right click is detected move the goal to the closest square in the grid
 *
 */
void BasicDrawPane::rightClick(wxMouseEvent& event)
{
	while(getAction() != 100) {
		step();
	}

	// Redraw the window
	Refresh();
}

void BasicDrawPane::step()
{
	// Get the next action
	int action = getAction();
	printf("Action %d\n", action);
	if(action != 100) {
		switch(action) {
					case 0:		// If the action is 0, move right
						SimulatorInterface::ROBOT_X += 1;
						break;
					case 2:		// If the action is 2, move left
						SimulatorInterface::ROBOT_X -= 1;
						break;
					case 1:		// If the action is 1, move up
						SimulatorInterface::ROBOT_Y -= 1;
						break;
					case 3:		// If the action is 3, move down
						SimulatorInterface::ROBOT_Y += 1;
						break;
		}

		increaseActionCounter();	
	}
}


/*
 * Catch keyboard inputs.
 * The robot can me moved with W,A,S,D and the goal with the arrows.
 */

void BasicDrawPane::keyPressed(wxKeyEvent& event) 
{
	wxChar uc = event.GetUnicodeKey();
    if ( uc != WXK_NONE )
    {
        // It's a "normal" character. Notice that this includes
        // control characters in 1..31 range, e.g. WXK_RETURN or
        // WXK_BACK, so check for them explicitly.
        if ( uc >= 32 )
        {
            switch(uc) {
				case 'A':
					SimulatorInterface::ROBOT_X -= 1;
					break;
				case 'D':
					SimulatorInterface::ROBOT_X += 1;
					break;
				case 'W':
					SimulatorInterface::ROBOT_Y -= 1;
					break;
				case 'S':
					SimulatorInterface::ROBOT_Y += 1;
					break;
			}
        }
        else
        {
            // It's a control character
        }
    }
    else // No Unicode equivalent.
    {
        // It's a special key, deal with all the known ones:
        switch ( event.GetKeyCode() )
        {
            case WXK_LEFT:
		SimulatorInterface::GOAL_X -= 1;				
		break;
            case WXK_RIGHT:
                SimulatorInterface::GOAL_X += 1;
                break;			
            case WXK_UP:
                SimulatorInterface::GOAL_Y -= 1;
                break;
            case WXK_DOWN:
                SimulatorInterface::GOAL_Y += 1;
                break;
        }
    }

	Refresh();
}

 
/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
 
void BasicDrawPane::paintEvent(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    render(dc);
}
 
/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 *
 * In most cases, this will not be needed at all; simply handling
 * paint events and calling Refresh() when a refresh is needed
 * will do the job.
 */
void BasicDrawPane::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}
 
/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void BasicDrawPane::render(wxDC&  dc)
{
	wxCoord width = 0;
	wxCoord height = 0;

    dc.GetSize(&width, &height);
	

	// Draw the grid
    dc.SetPen( wxPen( wxColor(0,0,0), 2 ) ); // black line, 3 pixels thick 

		
	for(wxCoord i = 0; i < width/stepSize; i++)
	{
		// draw a line
		dc.DrawLine( i*stepSize, 0, i*stepSize, height ); // draw vertical lines
	}	

	for(wxCoord i = 0; i < height/stepSize; i++)
	{
		// draw a line
		dc.DrawLine( 0, i*stepSize, width, i*stepSize ); // draw horizontal lines
	}	

	// Check the position of the robot and the goal
	checkPosition(width, height);

    // draw the goal
    dc.SetBrush(*wxCYAN_BRUSH); // blue filling
    dc.SetPen( wxPen( wxColor(255,175,175), 1 ) ); // 10-pixels-thick pink outline
    dc.DrawRectangle( stepSize*SimulatorInterface::GOAL_X, stepSize*SimulatorInterface::GOAL_Y, stepSize, stepSize);  // (x,y,w,h)

    dc.SetPen( wxPen( wxColor(0,0,0), 2 ) ); // black line, 3 pixels thick 
    dc.DrawText(wxT("G"), stepSize*SimulatorInterface::GOAL_X+stepSize/2-stepSize/8, stepSize*SimulatorInterface::GOAL_Y+stepSize/2-stepSize/4); 


    // draw the robot
    dc.SetBrush(*wxGREEN_BRUSH); // green filling
    dc.SetPen( wxPen( wxColor(0,0,180), 1 ) ); // 5-pixels-thick red outline
    dc.DrawCircle( wxPoint(stepSize*SimulatorInterface::ROBOT_X+stepSize/2,stepSize*SimulatorInterface::ROBOT_Y+stepSize/2), stepSize/2 /* radius */ );

    dc.SetPen( wxPen( wxColor(0,0,0), 2 ) ); // black line, 3 pixels thick 
    dc.DrawText(wxT("R"), stepSize*SimulatorInterface::ROBOT_X+stepSize/2-stepSize/8, stepSize*SimulatorInterface::ROBOT_Y+stepSize/2-stepSize/4); 

	// draw action
	std::string actionText = getActionText();
	wxFont font(18, wxDEFAULT, wxNORMAL, wxBOLD);	
	dc.SetFont(font);
	dc.SetTextForeground(wxColor(0,100,0)); // Color for the text
	dc.DrawText(wxString(actionText), 10, 10);
}

void BasicDrawPane::checkPosition(wxCoord w, wxCoord h)
{
	w /= stepSize;
	h /= stepSize;
	
	SimulatorInterface::ROBOT_X = SimulatorInterface::ROBOT_X >= w? w-1 : SimulatorInterface::ROBOT_X;
	SimulatorInterface::ROBOT_X = SimulatorInterface::ROBOT_X < 0? 0 : SimulatorInterface::ROBOT_X;

	SimulatorInterface::ROBOT_Y = SimulatorInterface::ROBOT_Y >= h? h-1 : SimulatorInterface::ROBOT_Y;
	SimulatorInterface::ROBOT_Y = SimulatorInterface::ROBOT_Y < 0? 0 : SimulatorInterface::ROBOT_Y;

	SimulatorInterface::GOAL_X = SimulatorInterface::GOAL_X >= w? w-1 : SimulatorInterface::GOAL_X;
	SimulatorInterface::GOAL_X = SimulatorInterface::GOAL_X < 0? 0 : SimulatorInterface::GOAL_X;

	SimulatorInterface::GOAL_Y = SimulatorInterface::GOAL_Y >= h? h-1 : SimulatorInterface::GOAL_Y;
	SimulatorInterface::GOAL_Y = SimulatorInterface::GOAL_Y < 0? 0 : SimulatorInterface::GOAL_Y;
}


/*
* Gets the string representation of the curernt action
*/
std::string BasicDrawPane::getActionText() {
	std::string base("Next Action: ");	
	switch(getAction()) {
		case 0:
			return base+"Move Right";
		case 2:
			return base+"Move Left";
		case 1:
			return base+"Move Up";
		case 3:
			return base+"Move Down";
		case 100:		// The code 100 is returned when there are no actions left
			return "Finished";
	}
	
	return "Unkown Action - "+getAction();
}

/*
* Gets the current action in the actions vector
*/
int BasicDrawPane::getAction() {
	printf("Current = %d, size = %d", currentAction, actions->size());
	if(currentAction == actions->size())
		return 100;
	return (*actions)[currentAction];
}

/*
* Increases the action counter
*/
void BasicDrawPane::increaseActionCounter() {
	currentAction++;
}

