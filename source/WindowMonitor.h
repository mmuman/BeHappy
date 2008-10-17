/*
 * BeHappy
 *
 * Par Sylvain Tertois
 *
 * Thread de gestion de fenêtres
 * Sert entres autres à bien gérer la fermeture de BeHappy tranquillement sans qu'il 
 * y ait de problèmes
 *
 */
 
 #ifndef WINDOWMONITOR_H
 #define WINDOWMONITOR_H
 
 #include <Looper.h>
  
 class BMessageRunner;
 
 class WindowMonitor : public BLooper
 {
 public:
 	WindowMonitor();
 	~WindowMonitor();
 	
 	void MessageReceived(BMessage *message);
 	void BroadcastMessageToWindows(int32 command);
 	void BroadcastMessageToWindows(BMessage *message);
 	
 	inline unsigned int GetNumWindows() { return m_NumWindows; }
 	
 	void CloseAll();
 
 private:
 	unsigned int m_NumWindows;
 	BMessageRunner *m_QuitTimeOut;	// timeout pour la fermeture de l'application
};

#endif //WINDOWMONITOR_H
