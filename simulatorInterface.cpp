#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/init.h>
#include <wx/app.h>
#include <wx/font.h>
#include <wx/utils.h>
#include "simulatorInterface.h"
#include "network.h"
#include <math.h>
#include <stdio.h>
#include <vector>

// WXWIDGETS Window Related Variables
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

// Simulation related variables
wxCoord stepSize = 40;
wxCoord InitialRobotX = 9;
wxCoord InitialRobotY = 6;
wxCoord goalX = 2;
wxCoord goalY = 10;

// Fitness related variables
float StartingFitness = 10.0;
float actionReward = 0.02;
float facingReward = 0.05;
float distanceReward = 0.15;


// Define the static member variables for ROBOT and GOAL POSITION
wxCoord SimulatorInterface::ROBOT_X = InitialRobotX;
wxCoord SimulatorInterface::ROBOT_Y = InitialRobotY;
wxCoord SimulatorInterface::GOAL_X = goalX;
wxCoord SimulatorInterface::GOAL_Y = goalY;

wxCoord SimulatorInterface::ROBOT_X_TMP = InitialRobotX;
wxCoord SimulatorInterface::ROBOT_Y_TMP = InitialRobotY;
wxCoord SimulatorInterface::GOAL_X_TMP = goalX;
wxCoord SimulatorInterface::GOAL_Y_TMP = goalY;

wxCoord SimulatorInterface::INITIAL_ROBOT_X = InitialRobotX;
wxCoord SimulatorInterface::INITIAL_ROBOT_Y = InitialRobotY;

// Define the static member varaibles for ROBOT Facing direction
Direction SimulatorInterface::DIRECTION = RIGHT;
Direction SimulatorInterface::DIRECTION_TMP = RIGHT;
bool SimulatorInterface::wasFacingGoal = false;
float SimulatorInterface::prevDistance = 0.0;
float SimulatorInterface::fitness_value = StartingFitness;


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
SimulatorInterface::SimulatorInterface(Network *net)
{
	network = net;
	stepCount = 0;
	ROBOT_X = InitialRobotX;
	ROBOT_Y = InitialRobotY;
	GOAL_X = goalX;
	GOAL_Y = goalY;


	ROBOT_X_TMP = InitialRobotX;
	ROBOT_Y_TMP = InitialRobotY;
	GOAL_X_TMP = goalX;
	GOAL_Y_TMP = goalY;

	INITIAL_ROBOT_X = InitialRobotX;
	INITIAL_ROBOT_Y = InitialRobotY;

	wxCoord x_ini = GOAL_X - INITIAL_ROBOT_X;
    wxCoord y_ini = GOAL_Y - INITIAL_ROBOT_Y;
    float distance = sqrt(x_ini*x_ini + y_ini*y_ini);

	DIRECTION = RIGHT;
	DIRECTION_TMP = RIGHT;
	wasFacingGoal = conditionMet(GOAL_VISIBLE);
	prevDistance = distance;
	fitness_value = StartingFitness;
}

/*
*	Runs the simulation without displaying it
*/
/*void SimulatorInterface::runSimulation() {	
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
} */


void SimulatorInterface::stepForward() {
	switch(SimulatorInterface::DIRECTION) {
		case RIGHT: SimulatorInterface::ROBOT_X += 1; break;
		case UP: SimulatorInterface::ROBOT_Y -= 1; break;
		case LEFT: SimulatorInterface::ROBOT_X -= 1; break;
		case DOWN: SimulatorInterface::ROBOT_Y += 1; break;
	}
}

bool SimulatorInterface::conditionMet(int condition) {
	switch(condition) {
		case NO_CONDITION: return true;
		case GOAL_VISIBLE: // Check if the goal is visible (90 degrees field of view)
			float distanceX = SimulatorInterface::GOAL_X - SimulatorInterface::ROBOT_X;	// If the distance is positive, then the goal is to the right, otherwise to the left
			float distanceY = SimulatorInterface::GOAL_Y - SimulatorInterface::ROBOT_Y; // If the distance is positive, then the goal is below, otherwise above

			// Check the direction the robot is facing and compare the distances to see if the goal is within the FOV
			switch(SimulatorInterface::DIRECTION) {
				case RIGHT: if(distanceX > 0 && abs(distanceY) <= distanceX) return true; break;
				case UP: if(distanceY < 0 && abs(distanceX) <= -distanceY) return true; break;
				case LEFT: if(distanceX < 0 && abs(distanceY) <= -distanceX) return true; break;
				case DOWN: if(distanceY > 0 && abs(distanceX) <= distanceY) return true; break;
			}

			break;

	}
	return false;
}

bool SimulatorInterface::goalReached() {
	return (SimulatorInterface::ROBOT_X == SimulatorInterface::GOAL_X && SimulatorInterface::ROBOT_Y == SimulatorInterface::GOAL_Y);
}

void SimulatorInterface::execute(int action) {
	switch(action) {
			case ROTATE_90:		// Rotate 90 degrees left
				SimulatorInterface::DIRECTION = (Direction)((SimulatorInterface::DIRECTION + 1) % 4);
				break;
			case ROTATE_180:	// Rotate 180 degrees
				SimulatorInterface::DIRECTION = (Direction)((SimulatorInterface::DIRECTION + 2) % 4);
				break;
			case ROTATE_270:	// Rotate 270 degrees left
				SimulatorInterface::DIRECTION = (Direction)((SimulatorInterface::DIRECTION + 3) % 4);
				break;
			case STEP_FORWARD:	// If the action is 4, step forward
				stepForward();
				break;
		}
}

/*
*	Runs the simulation without displaying it
*/
float SimulatorInterface::runSimulation() {	

	int maxIterations = 50;
	bool fired = true;
	// Iterate for the max number of iterations or until no transition was fired
	for(int iteration = 0; iteration < maxIterations && fired && !goalReached(); iteration++) {
		fired = false;
		for(int i = 0; i < network->transitions.size() && !goalReached(); i++) {
			// If a transition is enabled, then fire it
			if(network->isEnabled(network->transitions[i]) && conditionMet(network->transitions[i]->condition)) {
				network->fire(network->transitions[i]);
				execute(network->transitions[i]->action_ID);
				actions.push_back(network->transitions[i]->action_ID);
				updateFitness();
				stepCount++;
				fired = true;			
			}
		}
	}

	if(goalReached()) {
		SimulatorInterface::fitness_value *= 3.0;
	}

	return SimulatorInterface::fitness_value;

}

/*
*	Opens a GUI that displays the simulation
*/
float SimulatorInterface::displaySimulation() {
	int argc = 0;
	char ** argv = 0;

	int maxIterations = 50;
	bool fired = true;
	// Iterate for the max number of iterations or until no transition was fired
	for(int iteration = 0; iteration < maxIterations && fired && !goalReached(); iteration++) {
		fired = false;
		for(int i = 0; i < network->transitions.size() && !goalReached(); i++) {
			// If a transition is enabled, then fire it
			if(network->isEnabled(network->transitions[i]) && conditionMet(network->transitions[i]->condition)) {
				network->fire(network->transitions[i]);
				actions.push_back(network->transitions[i]->action_ID);			
				fired = true;			
			}
		}
	}

	stepCount = actions.size();

	wxApp* simulator = new Simulator(&actions);
	wxApp::SetInstance(simulator);	
	wxEntry(argc, argv);
	wxEntryCleanup();


	if(goalReached()) {
		SimulatorInterface::fitness_value *= 3.0;
	}

	return SimulatorInterface::fitness_value;
}


// This is meant to be used by itself and not as a part of a simulation
void SimulatorInterface::showSolution() {
	int argc = 0;
	char ** argv = 0;

	saveAndResetValues();

	wxApp* simulator = new Simulator(&actions);
	wxApp::SetInstance(simulator);	
	wxEntry(argc, argv);
	wxEntryCleanup();	

	loadValues();
}

void SimulatorInterface::saveAndResetValues() {
		ROBOT_X_TMP = ROBOT_X;
		ROBOT_Y_TMP = ROBOT_Y;
		GOAL_X_TMP = GOAL_X;
		GOAL_Y_TMP = GOAL_Y;
		DIRECTION_TMP  = DIRECTION;

		ROBOT_X = InitialRobotX;
		ROBOT_Y = InitialRobotY;
		GOAL_X = goalX;
		GOAL_Y = goalY;
		DIRECTION = RIGHT;

}

void SimulatorInterface::loadValues() {
		ROBOT_X = ROBOT_X_TMP;
		ROBOT_Y = ROBOT_Y_TMP;
		GOAL_X = GOAL_X_TMP;
		GOAL_Y = GOAL_Y_TMP;
		DIRECTION = DIRECTION_TMP;
}

/*
*	updates the fitness value as the simulator runs
*/
void SimulatorInterface::updateFitness() {

	float distanceX = SimulatorInterface::GOAL_X - SimulatorInterface::ROBOT_X;     // If the distance is positive, then the goal is to the right, otherwise to the left
    float distanceY = SimulatorInterface::GOAL_Y - SimulatorInterface::ROBOT_Y; // If the distance is positive, then the goal is below, otherwise above
	float distance = sqrt(distanceX*distanceX + distanceY*distanceY);
	bool facingGoal = conditionMet(GOAL_VISIBLE);

	// Give a negative reward for taking one more action
	SimulatorInterface::fitness_value -= actionReward;

	// Give a positive reward if the distance to the goal is shorter than the distance during the previous action
	if(distance < prevDistance)
		SimulatorInterface::fitness_value += distanceReward;
	// Give a negative reward if the distance to the goal is greater than the distance during the previous action
	else if(distance > prevDistance)
		SimulatorInterface::fitness_value -= distanceReward;
	// Give a small positive reward if we were not facing the goal and now we are
	if(!wasFacingGoal && facingGoal)
		SimulatorInterface::fitness_value += facingReward;
	// Give a small negative reward if we were facing the goal and now we're not
	else if(wasFacingGoal && !facingGoal)
		SimulatorInterface::fitness_value -= facingReward;


	// Update the distance
	SimulatorInterface::prevDistance = distance;
	// Update the goal facing variable
	SimulatorInterface::wasFacingGoal = facingGoal;
}


float SimulatorInterface::getFitnessValue() {
	// Calculate the initial distance between the robot and the goal
	wxCoord x_ini = SimulatorInterface::GOAL_X - SimulatorInterface::INITIAL_ROBOT_X;
	wxCoord y_ini = SimulatorInterface::GOAL_Y - SimulatorInterface::INITIAL_ROBOT_Y;
	float distance_ini = sqrt(x_ini*x_ini + y_ini*y_ini); // Not used at the moment

	// Calculate the minimum number of steps needed to reach the goal (can't make any diagonal movements)
	float min_steps = abs(x_ini) + abs(y_ini);	

	// Calculate the distance between the finishing position of the robot and the goal
	wxCoord x = SimulatorInterface::GOAL_X - SimulatorInterface::ROBOT_X;
	wxCoord y = SimulatorInterface::GOAL_Y - SimulatorInterface::ROBOT_Y;
	float distance = sqrt(x*x + y*y);	

	float steps = stepCount;

	// Add modifiers to play with the weight of the distance/steps
	float stepModifier = 1.0;
	float distanceModifier = 3.0;

	steps *= stepModifier;
	distance *= distanceModifier;

	float fitness = 1.0 / (1.0 + steps + distance); 

	if(conditionMet(1)) fitness *= 2.0;

	if(distance == 0.0) {
		// The goal was reached exactly, give an additional reward
		fitness *= 2.0;
	}

	return fitness;
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
//EVT_KEY_DOWN(BasicDrawPane::keyPressed)
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
	SimulatorInterface::updateFitness();

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
		SimulatorInterface::updateFitness();
	}

	// Redraw the window
	Refresh();
}

void BasicDrawPane::step()
{
	// Get the next action
	int action = getAction();
	if(action != 100) {
		/*switch(action) {
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
		}*/

		SimulatorInterface::execute(action);

		increaseActionCounter();	
	}
}


/*
 * Catch keyboard inputs.
 * The robot can me moved with W,A,S,D and the goal with the arrows.
 */

/*void BasicDrawPane::keyPressed(wxKeyEvent& event) 
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
            case WXK_ESCAPE:
            	// Clos
            	break;
        }
    }

	Refresh();
} */

 
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
	//checkPosition(width, height);

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

    switch(SimulatorInterface::DIRECTION) {
    	case RIGHT: dc.DrawCircle( wxPoint(stepSize*SimulatorInterface::ROBOT_X+stepSize/2+stepSize/4,stepSize*SimulatorInterface::ROBOT_Y+stepSize/2), stepSize/8 /* radius */ ); break;
    	case UP: dc.DrawCircle( wxPoint(stepSize*SimulatorInterface::ROBOT_X+stepSize/2,stepSize*SimulatorInterface::ROBOT_Y+stepSize/2-stepSize/4), stepSize/8 /* radius */ ); break;
    	case LEFT: dc.DrawCircle( wxPoint(stepSize*SimulatorInterface::ROBOT_X+stepSize/2-stepSize/4,stepSize*SimulatorInterface::ROBOT_Y+stepSize/2), stepSize/8 /* radius */ ); break;
    	case DOWN: dc.DrawCircle( wxPoint(stepSize*SimulatorInterface::ROBOT_X+stepSize/2,stepSize*SimulatorInterface::ROBOT_Y+stepSize/2+stepSize/4), stepSize/8 /* radius */ ); break;
    }

	// draw action
	std::string actionText = getActionText();
	wxFont font(18, wxDEFAULT, wxNORMAL, wxBOLD);	
	dc.SetFont(font);
	dc.SetTextForeground(wxColor(0,100,0)); // Color for the text
	dc.DrawText(wxString(actionText.c_str(), wxConvUTF8), 10, 10);
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
		case ROTATE_90:
			return base+"Rotate 90 degrees";
		case ROTATE_180:
			return base+"Rotate 180 degrees";
		case ROTATE_270:
			return base+"Rotate 270 degrees";
		case STEP_FORWARD:
			return base+"Step Forward";
		case 100:		// The code 100 is returned when there are no actions left
			return "Finished";
	}
	
	return "Unknown action";
}

/*
* Gets the current action in the actions vector
*/
int BasicDrawPane::getAction() {
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

