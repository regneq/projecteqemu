#ifndef EQEMU_OPTIONS_H
#define EQEMU_OPTIONS_H

/**
 * Collects options on one object, because having a bunch of global variables floating around is 
 * really ugly and just a little dangerous.
 */
class Options
{
public:
	/**
	 * Constructor, sets the default options.
	 */
	Options() : allow_unregistered(true), trace(false), dump_in_packets(false), dump_out_packets(false), encryption_mode(5) { }

	/**
	 * Sets allow_unregistered.
	 */
	inline void AllowUnregistered(bool b) { allow_unregistered = b; }

	/**
	 * Returns the value of allow_unregistered.
	 */
	inline bool IsUnregisteredAllowed() const { return allow_unregistered; }

	/**
	 * Sets trace.
	 */
	inline void Trace(bool b) { trace = b; }

	/**
	 * Returns the value of trace.
	 */
	inline bool IsTraceOn() const { return trace; }

	/**
	 * Sets dump_in_packets.
	 */
	inline void DumpInPackets(bool b) { dump_in_packets = b; }

	/**
	 * Returns the value of dump_in_packets.
	 */
	inline bool IsDumpInPacketsOn() const { return dump_in_packets; }

	/**
	 * Sets dump_out_packets.
	 */
	inline void DumpOutPackets(bool b) { dump_out_packets = b; }

	/**
	 * Returns the value of dump_out_packets.
	 */
	inline bool IsDumpOutPacketsOn() const { return dump_out_packets; }

	/**
	 * Sets encryption_mode.
	 */
	inline void EncryptionMode(int m) { encryption_mode = m; }

	/**
	 * Returns the value of encryption_mode.
	 */
	inline int GetEncryptionMode() const { return encryption_mode; }

	/**
	 * Sets local_network.
	 */
	inline void LocalNetwork(string n) { local_network = n; }

	/**
	 * Return the value of local_network.
	 */
	inline string GetLocalNetwork() const { return local_network; }

	/**
	 * Sets account table.
	 */
	inline void AccountTable(string t) { account_table = t; }

	/**
	 * Return the value of local_network.
	 */
	inline string GetAccountTable() const { return account_table; }

	/**
	 * Sets world account table.
	 */
	inline void WorldRegistrationTable(string t) { world_registration_table = t; }

	/**
	 * Return the value of world account table.
	 */
	inline string GetWorldRegistrationTable() const { return world_registration_table; }

	/**
	 * Sets world admin account table.
	 */
	inline void WorldAdminRegistrationTable(string t) { world_admin_registration_table = t; }

	/**
	 * Return the value of world admin account table.
	 */
	inline string GetWorldAdminRegistrationTable() const { return world_admin_registration_table; }

	/**
	 * Sets world server type table.
	 */
	inline void WorldServerTypeTable(string t) { world_server_type_table = t; }

	/**
	 * Return the value of world admin account table.
	 */
	inline string GetWorldServerTypeTable() const { return world_server_type_table; }

private:
	bool allow_unregistered;
	bool trace;
	bool dump_in_packets;
	bool dump_out_packets;
	int encryption_mode;
	string local_network;
	string account_table;
	string world_registration_table;
	string world_admin_registration_table;
	string world_server_type_table;
};

#endif

