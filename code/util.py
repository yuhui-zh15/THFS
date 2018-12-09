#encoding=utf-8
import urllib.request, urllib.error, urllib.parse
import http.cookiejar
from html.parser import HTMLParser
from html.entities import entitydefs
import httplib2


def GetOpener(user, password):
    url = "http://learn.tsinghua.edu.cn/MultiLanguage/lesson/teacher/loginteacher.jsp"
    opener = urllib.request.build_opener(urllib.request.HTTPCookieProcessor(http.cookiejar.CookieJar()))
    opener.addheaders = [('User-agent','Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)')]
    data = urllib.parse.urlencode({"userid": user, "userpass": password}).encode('utf-8')
    opener.open(url, data)
    return opener


class courseHTMLParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.flag = False
        self.id = []
        self.contains = []
    
    def handle_starttag(self, tag, attrs):
        if tag == 'a':
            if len(attrs) == 0 : 
                pass
            else:
                for (variable, value) in attrs:
                    if variable == "href" and value.find('course_id') != -1:
                        self.id.append(value[value.find('course_id') + 10:])
                        self.flag = True
                        
    def handle_data(self,data):
        if self.flag:
            self.contains.append(data.strip().replace('/', ' ').split('(')[0])
            self.contains.append(self.id.pop().strip())
        
    def handle_endtag(self, tag):
        if tag == "a":
            self.flag = False


class noteHTMLParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.flag = False
        self.links = []
        self.contains = ''
    
    def handle_starttag(self, tag, attrs):
        if tag == "td":
            self.flag = True    
    
    def handle_endtag(self, tag):
        if tag == "td" and self.flag:
            self.flag = False
            self.contains += '\n'
                        
    def handle_data(self, data):
        if self.flag:
            self.contains += data.strip()
    
    def handle_entityref(self, name): 
        if self.flag :
            if name == 'nbsp':
                self.contains += ' '
            elif name in entitydefs:
                self.contains += entitydefs[name]
            else:
                self.contains += ' '


class noteListHTMLParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.flag = False
        self.flag2 = False
        self.links = []
        self.contains = []
 
    def handle_starttag(self, tag, attrs):
        if tag == "a":
            if len(attrs) == 0: pass
            else:
                for (variable, value)  in attrs:
                    if variable == "href":
                        self.links.append('http://learn.tsinghua.edu.cn/MultiLanguage/public/bbs/' + value)
                        self.flag = True
    
    def handle_endtag(self, tag):
        if tag == "a" and self.flag:
            self.flag = False
            self.flag2 = False
    
    def handle_data(self, data):
        if self.flag and len(data.strip()):
            if self.flag2 :
                self.contains.append(self.contains.pop() + data.strip().replace('/', ' '))
            else:
                self.contains.append(data.strip().replace('/', ' '))
            self.flag2 = True
            
    def handle_entityref(self, name): 
        if self.flag and self.flag2:
            if name == 'nbsp':
                c = self.contains.pop() + ' '
            elif name in entitydefs:
                c = self.contains.pop() + entitydefs[name]
            else:
                c = self.contains.pop() + ' '
            self.contains.append(c)


class infoHTMLParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.flag = False
        self.links = []
        self.contains = ''
 
    def handle_starttag(self, tag, attrs):
        if tag == "td":
            self.flag = True
    
    def handle_endtag(self, tag):
        if tag == "td" and self.flag:
            self.flag = False
            self.contains += '\n'
                        
    def handle_data(self,data):
        if self.flag:
            self.contains += data.strip() 


class fileHTMLParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.flag = False
        self.flag2 = False
        self.links = []
        self.contains = []
 
    def handle_starttag(self, tag, attrs):
        self.flag = False
        self.flag2 = False
        if tag == "a":
            isDownload = False
            if len(attrs) == 0: pass
            else:
                for (variable, value)  in attrs:
                    if variable == "target" and value == "_top":
                        isDownload = True
                if not isDownload:
                    return
                for variable, value  in attrs:
                    if variable == "href":
                        self.links.append('http://learn.tsinghua.edu.cn' + value)
                        self.flag = True
                        
    def handle_data(self, data):
        if self.flag and len(data.strip()):
            if self.flag2 :
                self.contains.append(self.contains.pop() + data.strip().replace('/', ' '))
            else:
                self.contains.append(data.strip().replace('/', ' '))
            self.flag2 = True
            
    def handle_entityref(self, contains): 
        if self.flag :
            if self.flag2:
                tmp = self.contains.pop() + ' '
            self.contains.append(tmp)


def GetUserCourse(user, password):
    url = 'http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/MyCourse.jsp?language=cn'
    opener = GetOpener(user, password)
    data = opener.open(url).read().decode('utf-8')
    parser = courseHTMLParser()
    parser.feed(data)
    parser.close()
    return parser.contains


def GetCourseNote(path, courseid, user, password):
    url = 'http://learn.tsinghua.edu.cn/MultiLanguage/public/bbs/getnoteid_student.jsp?course_id=' + courseid
    opener = GetOpener(user, password)
    data = opener.open(url).read().decode('utf-8')
    parser = noteListHTMLParser()
    parser.feed(data)
    parser.close()
    ret = []
    for i, noteName in enumerate(parser.contains):
        data = opener.open(urllib.parse.quote(parser.links[i], safe='/:?=&%')).read().decode('utf-8')
        parser_ = noteHTMLParser()
        parser_.feed(data)
        parser_.close()
        ret.append(path + noteName)
        ret.append(noteName)
        ret.append(len(parser_.contains.encode('utf-8')))  
        with open(path + noteName, 'wb') as f:
            f.write(parser_.contains.encode('utf-8'))        
    return ret


def GetCourseInfo(courseid, user, password):
    url = 'http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/course_info.jsp?course_id=' + courseid
    opener = GetOpener(user, password)
    data = opener.open(url).read().decode('utf-8')
    parser = infoHTMLParser()
    parser.feed(data)
    parser.close()
    ret = []
    ret.append(len(parser.contains))
    ret.append(parser.contains)
    return ret


def GetCourseFile(courseid, user, password):
    url = 'http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/download.jsp?course_id=' + courseid
    opener = GetOpener(user, password)
    data = opener.open(url).read().decode('utf-8')
    parser = fileHTMLParser()
    parser.feed(data)
    parser.close()
    ret = []
    for i, fileName in enumerate(parser.contains):
        meta = opener.open(parser.links[i]).info() 
        file_type = '.' + (meta.get("Content-Disposition")[:-1].split('.')[-1])
        file_size = int(meta.get("Content-Length")) 
        ret.append(parser.links[i])
        ret.append(file_type)
        ret.append(file_size)
        ret.append(fileName + file_type)
    return ret


def DownloadFile(path, url, user, password):
    opener = GetOpener(user, password).open(url)
    with open(path, 'wb') as f: 
        while True:  
            data = opener.read(8192)  
            if not data: break  
            f.write(data)  
    return path 
    

def UploadFile(path, user, password, submit_id='830881', post_rec_id='5783996', course_id='154218'):
    data = open(path, 'rb').read()
    login_url = 'http://learn.tsinghua.edu.cn/MultiLanguage/lesson/teacher/loginteacher.jsp'
    handin_url = 'http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/hom_wk_handin.jsp'
    http = httplib2.Http()

    login_data = {
        "userid": user, 
        "userpass": password
    }
    login_header = {
        'Content-type': 'application/x-www-form-urlencoded', 
        'User-agent': 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)'
    }
    logged_response, content = http.request(login_url, 'POST', headers=login_header, body=urllib.parse.urlencode(login_data).encode('utf-8'))

    submit_data = {  
        "saveDir" : course_id + '/homewkrec/',
        'post_id' : submit_id,
        'post_rec_id' : post_rec_id,
        'course_id' : course_id,
        'post_rec_homewk_detail' : data,
        'url_post' : '/MultiLanguage/lesson/student/hom_wk_handin.jsp',
    }
    submit_header = {   
        'Content-Type': 'application/x-www-form-urlencoded', 
        'User-agent': 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)', 
        'Refer': 'http://learn.tsinghua.edu.cn/uploadFile/uploadFile.jsp',
        'Cookie': logged_response['set-cookie']
    }
    response, content = http.request(handin_url, 'POST', headers=submit_header, body=urllib.parse.urlencode(submit_data).encode('utf-8'))
  

if __name__ == '__main__':
    pass
