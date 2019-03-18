/****************************************************************
CNCRemote server
Copyright 2017-2018 Stable Design <les@sheetcam.com>


This program is free software; you can redistribute it and/or modify
it under the terms of the Mozilla Public License Version 2.0 or later
as published by the Mozilla foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License for more details.

You should have received a copy of the Mozilla Public License
along with this program; if not, you can obtain a copy from mozilla.org
******************************************************************/


#include "cncserver.h"
#include <sstream>

#include "timer.h"
#include "callback.h"
#include <iostream>



namespace CncRemote
{

enum { MAX_THREADS = 8 };



class Server::Handler : public linear::Handler {
public:
	Handler() {}
	~Handler() {}


		void OnConnect(const linear::Socket& socket) {
		const linear::Addrinfo& info = socket.GetPeerInfo();
		std::cout << "OnConnect: " << info.addr << ":" << info.port << std::endl;
	}

	void OnDisconnect(const linear::Socket& socket, const linear::Error&) {
		const linear::Addrinfo& info = socket.GetPeerInfo();
		std::cout << "OnDisconnect: " << info.addr << ":" << info.port << std::endl;
	}

	void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
		const linear::Addrinfo& info = socket.GetPeerInfo();
		switch(msg.type) {
		case linear::REQUEST:
		{
			linear::Request request = msg.as<linear::Request>();
			try
			{
				for (unsigned ct = 0; ct < m_requestHandlers.size(); ct++)
				{
					if (m_requestHandlers[ct]->GetName() == request.method)
					{
						m_requestHandlers[ct]->Execute(request, socket);
						return;
					}
				}
				linear::Response response(request.msgid, "", ExceptionData("Function not found", request.method));
				response.Send(socket);
			}
			catch (const std::exception& exc)
			{
				linear::Response response(request.msgid, "", ExceptionData(exc.what(), ""));
				response.Send(socket);
			}
		}
		break;

		case linear::RESPONSE:
		{
			linear::Response response = msg.as<linear::Response>();
			std::cout << "recv Response: msgid = " << response.msgid
				<< ", result = " << response.result.stringify()
				<< ", error = " << response.error.stringify()
				<< " from " << info.addr << ":" << info.port << std::endl;
			std::cout << "origin request: msgid = " << response.request.msgid
				<< ", method = \"" << response.request.method << "\""
				<< ", params = " << response.request.params.stringify() << std::endl;
		}
		break;

		case linear::NOTIFY:
		{
			try
			{
				linear::Notify notify = msg.as<linear::Notify>();
				for (unsigned ct = 0; ct < m_notifyHandlers.size(); ct++)
				{
					if (m_notifyHandlers[ct]->GetName() == notify.method)
					{
						m_notifyHandlers[ct]->Execute(notify, socket);
						return;
					}
				}
				linear::Notify response("_Exception_", ExceptionData("Method not found", notify.method));
				response.Send(socket);
			}
			catch (const std::exception& exc)
			{
				linear::Notify response("_Exception_", ExceptionData(exc.what(), ""));
				response.Send(socket);
			}
		}
		break;
		default:
		{
			std::cout << "BUG: please inform to linear-developers" << std::endl;
		}
		break;
		}
	}
	void OnError(const linear::Socket&, const linear::Message& msg, const linear::Error& err) {
		switch(msg.type) {
		case linear::REQUEST:
		{
			linear::Request request = msg.as<linear::Request>();
			std::cout << "Error to Send Request: msgid = " << request.msgid
				<< ", method = \"" << request.method << "\""
				<< ", params = " << request.params.stringify()
				<< ", err = " << err.Message() << std::endl;
		}
		break;
		case linear::RESPONSE:
		{
			linear::Response response = msg.as<linear::Response>();
			std::cout << "Error to Send Response: msgid = " << response.msgid
				<< ", result = " << response.result.stringify()
				<< ", error = " << response.error.stringify()
				<< ", err = " << err.Message() << std::endl;
		}
		break;
		case linear::NOTIFY:
		{
			linear::Notify notify = msg.as<linear::Notify>();
			std::cout << "Error to Send Notify: "
				<< "method = \"" << notify.method << "\""
				<< ", params = " << notify.params.stringify()
				<< ", err = " << err.Message() << std::endl;
		}
		break;
		default:
		{
			std::cout << "BUG: please inform to linear-developpers" << std::endl;
		}
		break;
		}
	}

	class HandlerCallback
	{
	public:
		virtual ~HandlerCallback() {}
		const string& GetName() {return m_name;}
	protected:
		void SetName(string n)
		{
			m_name = n;
			if (!m_name.empty() && m_name.at(m_name.size() - 1) == '_') //special case - remove trailing underscore
			{
				m_name.erase(m_name.size() - 1);
			}
		}

		string m_name;
	};

	class RequestHandler : public HandlerCallback
	{
	public:
		virtual void Execute(const linear::Request& req, const linear::Socket& socket) = 0;
	};



	template <class R>
	class RequestHandler0 : public RequestHandler
	{
	public:
		RequestHandler0(string name, const CBFunctor0wRet<R> func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Request& req, const linear::Socket& socket)
		{
			try
			{
				linear::Response response(req.msgid, m_func());
				response.Send(socket);
			}catch (const std::exception& exc)
			{
				linear::Response response(req.msgid, req.method, exc.what());
				response.Send(socket);
			}
		}

	private:
		CBFunctor0wRet<R> m_func;
	};


	template <class P, class R>
	class RequestHandler1 : public RequestHandler
	{
	public:
		RequestHandler1(string name, const CBFunctor1wRet<P, R> func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Request& req, const linear::Socket& socket)
		{
			try
			{
				linear::Response response(req.msgid, m_func(req.params.as<P>()));
				response.Send(socket);
			}catch (const std::bad_cast&)
			{
				linear::Response response(req.msgid, "", ExceptionData("Invalid arguments:" + req.params.stringify(), req.method));
				response.Send(socket);
			}catch (const std::exception& exc)
			{
				linear::Response response(req.msgid, "", ExceptionData(exc.what(), req.method));
				response.Send(socket);
			}
		}

	private:
		CBFunctor1wRet<P, R> m_func;
	};

	template <class P1, class P2, class R>
	class RequestHandler2 : public RequestHandler
	{
	public:
		RequestHandler2(string name, const CBFunctor2wRet<P1, P2, R> func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Request& req, const linear::Socket& socket)
		{
			try
			{
				Data d = req.params.as<Data>();
				linear::Response response(req.msgid, m_func(d.arg1, d.arg2));
				response.Send(socket);
			}catch (const std::bad_cast&)
			{
				linear::Response response(req.msgid, "", ExceptionData("Invalid arguments:" + req.params.stringify(), req.method));
				response.Send(socket);
			}catch (const std::exception& exc)
			{
				linear::Response response(req.msgid, "", ExceptionData(exc.what(), req.method));
				response.Send(socket);
			}
		}

		struct Data
		{
			P1 arg1;
			P2 arg2;
			MSGPACK_DEFINE_MAP(arg1, arg2);
		};

	private:
		CBFunctor2wRet<P1, P2, R> m_func;
	};



	class NotifyHandler : public HandlerCallback
	{
	public:
		virtual void Execute(const linear::Notify& req, const linear::Socket& socket) = 0;
	};


	class NotifyHandler0 : public NotifyHandler
	{
	public:
		NotifyHandler0(string name, const CBFunctor0 func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Notify& req, const linear::Socket& socket)
		{
			try
			{
				m_func();
			}catch (const std::exception& exc)
			{
				linear::Notify response("_Exception_", ExceptionData(exc.what(), req.method));
				response.Send(socket);
			}
		}

	private:
		CBFunctor0 m_func;
	};


	template <class P>
	class NotifyHandler1 : public NotifyHandler
	{
	public:
		NotifyHandler1(string name, const CBFunctor1<P> func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Notify& req, const linear::Socket& socket)
		{
			try
			{
				m_func(req.params.as<P>());
			}catch (const std::bad_cast&)
			{
				linear::Notify response("_Exception_", ExceptionData("Invalid arguments: " + req.params.stringify(), req.method));
				response.Send(socket);
			}catch (const std::exception& exc)
			{
				linear::Notify response("_Exception_", ExceptionData(exc.what(), req.method));
				response.Send(socket);
			}
		}

	private:
		CBFunctor1<P> m_func;
	};


	template <class P1, class P2>
	class NotifyHandler2 : public NotifyHandler
	{
	public:
		NotifyHandler2(string name, const CBFunctor2<P1, P2> func)
		{
			SetName(name);
			m_func = func;
		}

		virtual void Execute(const linear::Notify& req, const linear::Socket& socket)
		{
			try
			{
				Data d = req.params.as<Data>();
				m_func(d.arg1, d.arg2);
			}catch (const std::bad_cast&)
			{
				linear::Notify response("_Exception_", ExceptionData("Invalid arguments: " + req.params.stringify(), req.method));
				response.Send(socket);
			}catch (const std::exception& exc)
			{
				linear::Notify response("_Exception_", ExceptionData(exc.what(), req.method));
				response.Send(socket);
			}
		}

		struct Data
		{
			P1 arg1;
			P2 arg2;
			MSGPACK_DEFINE_MAP(arg1, arg2);
		};

	private:
		CBFunctor2<P1, P2> m_func;
	};
	/*
#define BIND_NOTIFY1(func, type)\
	class Class_##func : public Server::Handler::NotifyHandler\
	{\
	public:\
		Class_##func(string n, Server * svr){Init(n,svr);}\
		virtual void Execute(const linear::Notify& req, const linear::Socket& socket)\
		{\
			try{\
				m_server->func(req.as<type>());\
			}catch (const std::bad_cast&){\
				linear::Notify response("_Exception_", ErrorData("Invalid arguments", req.method));\
				response.Send(socket);\
			}catch (const std::exception& exc){\
				linear::Notify response("_Exception_", ErrorData(exc.what(), req.method));\
				response.Send(socket);\
			}\
		}\
	};\
	m_handler->BindNotify(new Class_##func(#func,this));

*/


	void BindRequest(RequestHandler * req)
	{
		m_requestHandlers.push_back(req);
	}

	void BindNotify(NotifyHandler * req)
	{
		m_notifyHandlers.push_back(req);
	}

private:
	std::vector<NotifyHandler *> m_notifyHandlers;
	std::vector<RequestHandler *> m_requestHandlers;
};


Server::Server()
{
	m_file = NULL;

	m_handler = linear::shared_ptr<Handler>(new Handler());
	m_server = linear::TCPServer(m_handler);

#define BIND_REQ0(R, func) m_handler->BindRequest(new Handler::RequestHandler0<R>(#func, makeFunctor((CBFunctor0wRet<R> *)0, *this, &Server::func)));
#define BIND_REQ1(R, func, A1) m_handler->BindRequest(new Handler::RequestHandler1<A1, R>(#func, makeFunctor((CBFunctor1wRet<A1, R> *)0, *this, &Server::func)));
#define BIND_REQ2(R, func, A1, A2) m_handler->BindRequest(new Handler::RequestHandler2<A1, A2, R>(#func, makeFunctor((CBFunctor2wRet<A1, A2, R> *)0, *this, &Server::func)));

#define BIND_NOTIFY0(func) m_handler->BindNotify(new Handler::NotifyHandler0(#func, makeFunctor((CBFunctor0 *)0, *this, &Server::func)));
#define BIND_NOTIFY1(func, A1) m_handler->BindNotify(new Handler::NotifyHandler1<A1>(#func, makeFunctor((CBFunctor1<A1> *)0, *this, &Server::func)));
#define BIND_NOTIFY2(func, A1, A2) m_handler->BindNotify(new Handler::NotifyHandler2<A1, A2>(#func, makeFunctor((CBFunctor2<A1, A2> *)0, *this, &Server::func)));


	BIND_REQ0(State, GetState_);
	BIND_NOTIFY1(DrivesOn, bool);
	BIND_NOTIFY1(JogVel, Axes);
	BIND_NOTIFY2(JogStep, Axes, double);
	BIND_REQ1(bool, Mdi, string);
	BIND_NOTIFY1(FeedOverride, double);
	BIND_NOTIFY1(SpindleOverride, double);
	BIND_NOTIFY1(RapidOverride, double);
	BIND_REQ1(bool, LoadFile, string);
	BIND_REQ0(bool, CloseFile);
	BIND_NOTIFY0(CycleStart);
	BIND_NOTIFY0(CycleStop);
	BIND_NOTIFY1(FeedHold, bool);
	BIND_NOTIFY1(BlockDelete, bool);
	BIND_NOTIFY1(SingleStep, bool);
	BIND_NOTIFY1(OptionalStop, bool);
	BIND_NOTIFY1(Home, BoolAxes);
	BIND_REQ1(Axes, GetOffset, unsigned int);
	BIND_REQ0(vector<int>, GetGCodes);
	BIND_REQ0(vector<int>, GetMCodes);

	BIND_REQ1(string, SendInit, string);
	BIND_REQ2(bool, SendData, string, int);
	BIND_REQ1(string, GetError, unsigned int);
	BIND_REQ1(string, GetMessage, unsigned int);
	BIND_REQ0(float, Version);
	BIND_REQ1(bool, StartPreview, int);
	BIND_REQ0(PreviewData, GetPreview);
	BIND_NOTIFY0(EndPreview);
}

Server::~Server()
{
	DeleteTemp();
}



COMERROR Server::Bind(const uint32_t port)
{
	m_server.Stop();
	m_server.Start("0.0.0.0", port);
	return errOK;
}

LockedState Server::GetState()
{
	LockedState s(m_state, m_syncLock);
	return s;
}

string Server::SendInit(string nameHint)
{
	CloseFile();
	DeleteTemp();
	m_curBlock = 0;
#ifdef _WIN32
	char szTempFileName[MAX_PATH];
	char lpTempPathBuffer[MAX_PATH];
	DWORD dwRetVal = GetTempPathA(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		return("");
	}

	//  Generates a temporary file name.
	UINT uRetVal = GetTempFileNameA(lpTempPathBuffer, nameHint.c_str(), 0, szTempFileName);
	if (uRetVal == 0)
	{
		return("");
	}
	m_file = fopen(szTempFileName, "w");
	if (m_file)
	{
		m_curFile = szTempFileName;
		return szTempFileName;
	}
	return("");
#else
	char path[PATH_MAX];
	sprintf(path,"/tmp/CncRemote%sXXXXXX", nameHint.c_str());
	int fd = mkstemp(path);
	if (fd < 0) return ("");
	m_file = fdopen(fd, "w");
	if (!m_file) return ("");
	m_curFile = path;
	return path;
#endif

}

bool Server::SendData(const string data, const int block)
{
	if (!m_file) return false;
	if (block != m_curBlock)
	{
		fclose(m_file);
		return false;
	}
	m_curBlock = block + 1;
	int l = data.size();
	int r = fwrite(data.c_str(), 1, l, m_file);
	if (r != l)
	{
		fclose(m_file);
		return false;
	}
	if (l < FILE_BLOCK_SIZE) fclose(m_file);
	return true;
}


void Server::DeleteTemp()
{
	if (m_file) fclose(m_file);
	if (m_curFile.empty()) return;
	remove(m_curFile.c_str());
}

State Server::GetState_()
{
    ThreadLock lock = GetLock();
	UpdateState(m_state);
	return State(m_state);
}

bool Server::LoadFile2(const string file)
{
	m_state.fileCount++;
	return LoadFile(file);
}

string Server::GetError(const unsigned int index)
{
	if (index >= m_errors.size()) return string();
	return m_errors[index];
}

string Server::GetMessage(const unsigned int index)
{
	if (index >= m_messages.size()) return string();
	return m_messages[index];
}

void Server::LogError(string error)
{
	if (error.size() > 0)
	{
		m_errors.push_back(error);
		m_state.errorCount = m_errors.size();
	}
}

void Server::LogMessage(string message)
{
	if (message.size() > 0)
	{
		m_messages.push_back(message);
		m_state.messageCount = m_messages.size();
	}
}




/*
void Connection::RemoveTemp()
{
	if(m_loadFile) fclose(m_loadFile);
	m_loadFile = NULL;
	if(m_tempFileName.size() > 0)
	{
#ifdef _WIN32
		_wremove(m_tempFileName.c_str());
#else
		remove(m_tempFileName.c_str());
#endif
		m_tempFileName.clear();
	}
}

void Connection::RecieveFileInit(const CmdBuf& cmd)
{
	RemoveTemp();
#ifdef _WIN32
	wchar_t buf[MAX_PATH];
	if(GetEnvironmentVariableW(_T("TEMP"), buf, MAX_PATH) == 0)
	{
		printf("Failed to find temp directory");
		return;
	}
	CncString path = buf;
	path += _T("\\CNCRemote");
	if(_wmkdir(path.c_str()) < 0 && errno != EEXIST)
	{
		printf("Failed to make temp directory");
		return;
	}
	path += _T('\\');
	path += from_utf8(cmd.string().c_str());
	m_loadFile = _wfopen(path.c_str(), _T("wb"));
	if(!m_loadFile)
	{
		printf("Failed to create temp file");
		return;
	}
	m_tempFileName = path;
	m_loadLength = cmd.intval();
	m_loadCount = 0;
#else
#endif
}

void Connection::RecieveFileData(const CmdBuf& cmd)
{
	if(!m_loadFile)
	{
		printf("Unexpected file data");
		return;
	}
	int len = cmd.string().size();
	fwrite(cmd.string().data(), 1, len, m_loadFile);
	m_loadCount += len;
	if(m_loadCount < m_loadLength)
	{
		return;
	}
	fclose(m_loadFile);
	m_loadFile = NULL;
	if(m_loadCount != m_loadLength)
	{
		printf("Received file is wrong length");
	}
}
*/

} //namespace CncRemote

