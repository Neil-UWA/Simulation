extern	bool compareAP(FRAME frame, double rxsignal);
extern	void addToAPList(FRAME frame, double rxsignal);
extern	int	 whichAP(void);
extern	void show(void);
extern	void init_searchAP(void);
extern	void init_timeout(void);
extern	void start_timeout(void);
extern	void init_talking(void);
extern	void start_talking(void);
extern	void stop_talking(void);

extern	EVENT_HANDLER(talk_to_ap);
extern	EVENT_HANDLER(searching_ap);
extern	EVENT_HANDLER(timeouts);
extern	EVENT_HANDLER(talking);
extern	EVENT_HANDLER(listen_to_ap);
