interface Deadline 
{ 
	async command void start(uint count); 
	async event void done(uint count); 
}