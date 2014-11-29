#ifndef SIMULATORINTERFACE_H
#define SIMULATORINTERFACE_H

#include <wx/wx.h>
#include <wx/sizer.h>
#include <vector>
#include <string>

class SimulatorInterface {
	public:
		void runSimulation();		// Runs the simulation internally and gets the fitness value
		void displaySimulation();	// Opens the simulator and shows the actual simulation
		float getFitnessValue();	// Returns the fitness value found by either running or displaying the simulation

		std::vector<int> *actions;
		int currentAction;
		
		static wxCoord ROBOT_X;
		static wxCoord ROBOT_Y;
		static wxCoord GOAL_X;
		static wxCoord GOAL_Y;

		SimulatorInterface(std::vector<int> *actions);
};
 
class BasicDrawPane : public wxPanel
{
 
public:
    BasicDrawPane(wxFrame* parent, std::vector<int> *actions);
 
    void paintEvent(wxPaintEvent & evt);
	void leftClick(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
    void keyPressed(wxKeyEvent& event);
    void paintNow();

	void checkPosition(wxCoord width, wxCoord height);

	std::string getActionText();		// Get the text for the current action
	int getAction();					// Get the current action in the vector actions	
	void	increaseActionCounter();		// Get the next action in the vector actions

	std::vector<int> *actions;			// Actions vector
	unsigned int currentAction;					// Index of the current action
 
    void render(wxDC& dc);
 
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
