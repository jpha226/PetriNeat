#ifndef SIMULATORINTERFACE_H
#define SIMULATORINTERFACE_H

#include <wx/wx.h>
#include <wx/sizer.h>
#include <vector>
#include <string>
#include "network.h"

using namespace NEAT;

enum Direction {
	RIGHT = 0,
	UP = 1,
	LEFT = 2,
	DOWN = 3,
};

enum Conditions {
	NO_CONDITION = 0,
	GOAL_VISIBLE = 1,
};

enum Actions {
	ROTATE_90 = 0,
	ROTATE_180 = 1,
	ROTATE_270 = 2,
	STEP_FORWARD = 3,
};

class SimulatorInterface {
	public:
		float runSimulation();						// Runs the simulation internally and gets the fitness value
		float displaySimulation();					// Opens the simulator and shows the actual simulation
		float getFitnessValue();					// Returns the fitness value found by either running or displaying the simulation
		void showSolution();						// Displays the solution
		static void stepForward();					// Checks the directions that the robot is facing and steps in that direction
		static bool conditionMet(int condition);	// Returns whether the given condition is met at this point in the simulation
		static void execute(int action);			// Executes an action in the network
		static bool goalReached();					// Returns weather the robot is in the goal state or not
		static void saveAndResetValues();			// Saves the values of the variables and restores them to their defaults
		static void loadValues();					// Load the values of the variables to their saved values


		Network *network;							// Network that contains the actions to be simulated
		int stepCount;								// Number of actions executed in the simulation
		static void updateFitness();		
		std::vector<int> actions;

		static wxCoord ROBOT_X;
		static wxCoord ROBOT_Y;
		static wxCoord GOAL_X;
		static wxCoord GOAL_Y;

		// Variables to hold the values when showing the simulation
		static wxCoord ROBOT_X_TMP;
		static wxCoord ROBOT_Y_TMP;
		static wxCoord GOAL_X_TMP;
		static wxCoord GOAL_Y_TMP;
		static Direction DIRECTION_TMP;

		static wxCoord INITIAL_ROBOT_X;
		static wxCoord INITIAL_ROBOT_Y;

		static Direction DIRECTION;
		static bool wasFacingGoal;
		static float prevDistance;
		static float fitness_value;

		SimulatorInterface(Network *net);
};
 
class BasicDrawPane : public wxPanel
{
 
public:
    BasicDrawPane(wxFrame* parent, std::vector<int> *actions);
    
    void paintEvent(wxPaintEvent & evt);
	void leftClick(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
    //void keyPressed(wxKeyEvent& event);
    void paintNow();

	void checkPosition(wxCoord width, wxCoord height);

	std::string getActionText();		// Get the text for the current action
	int getAction();					// Get the current action in the vector actions	
	void	increaseActionCounter();		// Get the next action in the vector actions

	std::vector<int> *actions;			// Actions vector
	unsigned int currentAction;					// Index of the current action

    void render(wxDC& dc);
	void step();
 
    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseDown(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyReleased(wxKeyEvent& event);
     */
 
    DECLARE_EVENT_TABLE()
};
 
 
class Simulator: public wxApp
{
    bool OnInit();
 
	std::vector<int> *actions;

    wxFrame *frame;
    BasicDrawPane * drawPane;
public:
	Simulator(std::vector<int> *actions);
};

#endif
