#include<iostream>
#include<string>
#include<ctime>
#include<map>
#include<sstream>
#include<list>
#include<algorithm>
using namespace std;

char disk[8192];
const int dataSize = 56;
const int nameLen = 16;
map<int, string> pass;
map<int, string> uid2name;
int curUser;
int curDir;
struct Finfo
{
	int length;
	int owner;
	int isDir;
	int firstSub;
	int nextCousin;
	char protectOwner;
	char protectOther;
	char next;
	char no_use;
	int open;
	int parent;
	__int64 createTime;
	__int64 modifyTime;
	__int64 visitTime;
	char name[nameLen];
	char data[dataSize];
};
Finfo newFile(string name,bool isDir)
{
	if (name.length() >= nameLen)
	{
		cout << "File name cannot be longer than " << nameLen - 1 << "." << endl;
		throw 0;
	}
	Finfo ret;
	ret.open = ret.next = ret.length = 0;
	ret.owner = curUser;
	ret.isDir = isDir;
	ret.createTime = ret.modifyTime = ret.visitTime = time(NULL);
	ret.firstSub = ret.parent = ret.nextCousin = -1;
	ret.protectOwner = 7;
	ret.protectOther = 5;
	memset(ret.data, 0, sizeof(ret.data));
	strcpy_s(ret.name, name.c_str());
	return ret;
}
int sz;
Finfo *block;
bool chkPriv(int id, char priv)
{
	if (block[curDir].owner == curUser)
	{
		if ((block[curDir].protectOwner & priv) == priv)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if ((block[curDir].protectOther & priv) == priv)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
void login()
{
	string user, password;
	cout << "Username:";
	cin >> user;
	cout << "Password:";
	cin >> password;
	int tuid=-1;
	for (auto i : uid2name)
	{
		if (i.second == user)
		{
			tuid = i.first;
		}
	}
	if (tuid==-1)
	{
		cout << "User not found." << endl;
		return;
	}
	if (password == pass[tuid])
	{
		curUser = tuid;
		cout << "Login success." << endl;
	}
	else
	{
		cout << "Wrong password." << endl;
	}
}
void welcome()
{
	cout << "MyDOS 1.0" << endl
		<< "Created by Shen Zeyu." << endl
		<< "Compile time " << __DATE__ << " " << __TIME__ << endl
		<< "All rights reserved." << endl << endl;
}
void showDiskInfo()
{
	cout << "Size of block is " << sizeof(Finfo) << " byte and mem size is " << sizeof(disk) << " ." << endl << endl;
}
void setUse(char *used, int pos, bool val)
{
	if (val)
	{
		used[pos / 8] = used[pos / 8] | ((char)1 << (pos % 8));
	}
	else
	{
		used[pos / 8] = used[pos / 8] & ~((char)1 << (pos % 8));
	}
}
bool getUse(char *used, int pos)
{
	return (used[pos / 8] & ((char)1 << (pos % 8))) != 0;
}
int findUnused(char *used)
{
	for (int i = 0; i < sz; i++)
	{
		if (!getUse(used, i))
		{
			return i;
		}
	}
	return -1;
}
int getFile(int cur, string target)
{
	if (target == ".")
		return curDir;
	if (target == "..")
		return block[cur].parent;
	int pos = block[cur].firstSub;
	while (pos != -1)
	{
		if (target == string(block[pos].name))
		{
			return pos;
		}
		pos = block[pos].nextCousin;
	}
	return - 1;
}
void chdir(string target)
{
	int pos = getFile(curDir, target);
	if (pos == -1)
	{
		cout << "Not found." << endl;
		return;
	}
	if (!block[pos].isDir)
	{
		cout << "Not directory." << endl;
		return;
	}
	if (chkPriv(pos, 1))
	{
		curDir = pos;
		block[curDir].visitTime = time(NULL);
	}
	else
	{
		cout << "Cannot access." << endl;
	}
	
}
void _make(string target,bool isDir)
{
	int nxt = findUnused(block->data);
	if (nxt==-1)
	{
		cout << "File system error, no free block found." << endl;
		return;
	}
	setUse(block[0].data, nxt, true);
	block[nxt] = newFile(target, isDir);
	block[nxt].parent = curDir;
	int pos = block[curDir].firstSub;
	if (pos == -1)
	{
		block[curDir].firstSub = nxt;
	}
	else if (strcmp(block[pos].name, block[nxt].name) > 0)
	{
		block[curDir].firstSub = nxt;
		block[nxt].nextCousin = pos;
	}
	else
	{
		int go;
		while (true)
		{
			go = block[pos].nextCousin;
			if (go == -1)
			{
				block[pos].nextCousin = nxt;
				break;
			}
			else if (strcmp(block[go].name, block[nxt].name) > 0)
			{
				block[pos].nextCousin = nxt;
				block[nxt].nextCousin = go;
				break;
			}
			else
			{
				pos = go;
			}
		}
	}
}
void mkdir(string target)
{
	if (chkPriv(curDir,2))
	{
		_make(target,true);
	}
	else
	{
		cout << "Cannot create." << endl;
	}
}
void initUser()
{
	int ufile = getFile(0, "user.txt");
	if (ufile!=-1 && block[ufile].isDir == false)
	{
		char *data = block[ufile].data;
		stringstream tmp(data);
		int n;
		tmp >> n;
		while(n--)
		{
			int uid;
			string name, password;
			tmp >> uid >> name >> password;
			pass[uid] = password;
			uid2name[uid] = name;
		}
	}
	else
	{
		pass[1000] = "root";
		uid2name[1000] = "root";
	}
	curUser = -1;
}
void create(string fname)
{
	if (chkPriv(curDir, 2))
	{
		_make(fname, false);
	}
	else
	{
		cout << "Cannot create." << endl;
	}
}
string getPath(int id)
{
	string path="";
	while (id != 0)
	{
		path = string(block[id].name) + "/" + path;
		id = block[id].parent;
	}
	path = "/" + path;
	return path;
}
string getTime(tm *t)
{
	char ret[20];
	sprintf_s(ret, "%04d-%02d-%02d %02d:%02d:%02d", (t->tm_year + 1990), (t->tm_mon + 1), (t->tm_mday), (t->tm_hour), (t->tm_min), (t->tm_sec));
	//max length is 20
	return ret;
}
string getPriv(char c)
{
	string ret = "";
	ret += (c & 4) ? "r" : "-";
	ret += (c & 2) ? "w" : "-";
	ret += (c & 1) ? "x" : "-";
	return ret;
}

void showDir()
{
	if (!chkPriv(curDir, 1))
	{
		cout << "Not permited." << endl;
		return;
	}
	cout << endl;
	tm tmp;
	localtime_s(&tmp, &block[curDir].modifyTime);
	printf("%-8s %-8s %-3dByte %c%-3s %-3s %-20s\n",
		".", 
		uid2name[block[curDir].owner].c_str(),
		block[curDir].length,
		block[curDir].isDir?'D':' ',
		getPriv(block[curDir].protectOwner).c_str(), 
		getPriv(block[curDir].protectOther).c_str(),
		getTime(&tmp).c_str()
	);
	int pos = block[curDir].firstSub;
	while (pos != -1)
	{
		Finfo &a = block[pos];
		localtime_s(&tmp, &a.modifyTime);
		printf("%-8s %-8s %-3dByte %c%-3s %-3s %-20s %#06x\n", 
			a.name,
			uid2name[a.owner].c_str(),
			a.length,
			a.isDir ? 'D' : ' ',
			getPriv(a.protectOwner).c_str(),
			getPriv(a.protectOther).c_str(),
			getTime(&tmp).c_str(),
			pos
		);
		pos = block[pos].nextCousin;
	}
	cout << endl;
}
void showPrompt()
{
	cout << uid2name[curUser] << "@" << getPath(curDir) << " # ";
}
void chmod(int a, int b)
{
	if (curUser == 1000 || block[curDir].owner == curUser)
	{
		block[curDir].protectOwner = a;
		block[curDir].protectOther = b;
	}
	else
	{
		cout << "No permition." << endl;
	}
}
void chown(string uname)
{
	int uid = -1;
	for (auto i : uid2name)
	{
		if (i.second == uname)
		{
			uid = i.first;
			break;
		}
	}
	if (uid == -1)
	{
		cout << "User not found." << endl;
		return;
	}
	if (curUser == 1000 || block[curDir].owner == curUser)
	{
		block[curDir].owner = curUser;
	}
	else
	{
		cout << "No permition." << endl;
	}
}
void _delSingle(int id)
{
	
	int parent = block[id].parent;
	if (block[parent].firstSub == id)
	{
		block[parent].firstSub = block[id].nextCousin;
		setUse(block[0].data, id, false);
	}
	else
	{
		int pos = block[parent].firstSub;
		while (block[pos].nextCousin != id)
		{
			pos = block[pos].nextCousin;
		}
		block[pos].firstSub = block[id].nextCousin;
		setUse(block[0].data, id, false);
	}
}
void _delSubTree(int id)
{
	int pos = block[id].firstSub;
	while (pos != -1)
	{
		int nxt = block[pos].nextCousin;
		_delSubTree(pos);
		pos = nxt;
	}
	if (block[id].owner == curUser)
	{
		if ((block[id].protectOwner & 3)==3)
		{
			_delSingle(pos);
		}
		else
		{
			cout << "Cannot delete \"" << block[pos].name << "\"." << endl;
		}
	}
	else
	{
		if ((block[id].protectOther & 3)==3)
		{
			_delSingle(pos);
		}
		else
		{
			cout << "Cannot delete \"" << block[pos].name << "\"." << endl;
		}
	}
}
void del(string fname)
{
	int tar = getFile(curDir, fname);
	if (tar == -1)
	{
		cout << "File not found." << endl;
	}
	if (block[tar].isDir)
	{
		cout << "Do you want to del the folder and ALL FILES in it?: ";
		string ans;
		cin >> ans;
		if (ans == "y" || ans == "Y")
		{
			_delSubTree(tar);
		}
	}
	else
	{
		_delSubTree(tar);
	}
}
int main()
{
	welcome();
	showDiskInfo();
	sz = sizeof(disk) / sizeof(Finfo);
	block = (Finfo*)disk;
	Finfo &root = *block;
	while (true)
	{
		cout << "Read data from file (Y/N)£¿: " ;
		string ans;
		cin >> ans;
		if (ans == "n" || ans == "N")
		{
			memset(disk, sizeof(disk), 0);
			block[0] = newFile("/", true);
			setUse(block[0].data, 0, true);
			break;
		}
		else if(ans == "y" || ans == "Y")
		{
			FILE *file;
			if (fopen_s(&file, "data.bin", "rb"))
			{
				cout << "data.bin not exists." << endl;
				break;
			}
			int pos = 0;
			while (pos < sizeof(disk))
			{
				int c = fgetc(file);
				if (c == EOF)
				{
					break;
				}
				disk[pos++] = c;
			}
			fclose(file);
			break;
		}
	}
	if (!root.isDir)
	{
		cout << "File system error!" << endl;
		exit(EXIT_FAILURE);
	}
	curDir = 0;
	initUser();

	while (curUser == -1)
	{
		login();
	}
	int home = getFile(0, uid2name[curUser]);
	if (home == -1)
	{
		home = 0;
	}
	curDir = home;
	while (true)
	{
		showPrompt();
		string cmd;
		cin >> cmd;
		if (cmd == "exit")
		{
			break;
		}
		else if (cmd == "cd")
		{
			string dir;
			cin >> dir;
			chdir(dir);
		}
		else if (cmd == "mkdir")
		{
			string dir;
			cin >> dir;
			mkdir(dir);
		}
		else if (cmd == "create")
		{
			string fname;
			cin >> fname;
			create(fname);
		}
		else if (cmd == "delete")
		{
			string fname;
			cin >> fname;
			del(fname);
		}
		else if (cmd == "login")
		{
			login();
		}
		else if (cmd == "dir")
		{
			showDir();
		}
		else if (cmd == "chmod")
		{
			int protectOwner, protectOther;
			cin >> protectOwner >> protectOther;
			chmod(protectOwner, protectOther);
		}
		else if (cmd == "chown")
		{
			string uname;
			cin >> uname;
			chown(uname);
		}
		else if (cmd == "cls")
		{
			system("cls");
		}
		else
		{
			cout << "Command not found." << endl;
		}
	}
	while (true)
	{
		cout << "Write data to file (Y/N)£¿: ";
		string ans;
		cin >> ans;
		if (ans == "n" || ans == "N")
		{
			break;
		}
		else if (ans == "y" || ans == "Y")
		{
			FILE *file;
			if (fopen_s(&file, "data.bin", "wb"))
			{
				cout << "Cannot create data.bin." << endl;
				break;
			}
			int pos = 0;
			while (pos < sizeof(disk))
			{
				if (fputc(disk[pos++], file) == EOF)
				{
					break;
				}
			}
			fclose(file);
			break;
		}
	}
	cout << "BYE!" << endl;
	return 0;
}