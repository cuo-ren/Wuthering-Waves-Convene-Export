import requests
import argparse
import json
import os

parser = argparse.ArgumentParser()
parser.add_argument('--url', type=str, required=True)
parser.add_argument('--type', type=str, required=True)
parser.add_argument('--headers', type=str)
parser.add_argument('--params', type=str)
parser.add_argument('--timeout', type=int, default=10)
args = parser.parse_args()


headers = json.loads(args.headers) if args.headers else {}

params = {}
if args.params:
    for item in args.params.split("&"):
        parts = item.split("=")
        if len(parts) != 2:
            print(f"ERROR:参数格式错误: {item}")
            exit(0)
            #continue
        key = parts[0]
        val_type = parts[1].split("|")
        if len(val_type) != 2:
            print(f"ERROR:参数类型格式错误: {item}")
            exit(0)
           # continue
        val, typ = val_type[0], val_type[1].lower()
        try:
            if typ == "int":
                params[key] = int(val)
            elif typ == "str":
                params[key] = val
            elif typ == "float":
                params[key] = float(val)
            else:
                print(f"ERROR:未处理的类型: {typ}")
                exit(0)
        except Exception as e:
            print(f"ERROR:参数转换失败: {item}, 错误: {e}")
            exit(0)

if args.type.lower() == "get":
    if params is None:
        params = ""
    r = requests.get(args.url + params, headers = headers)
    r.encoding = 'utf-8'
    print("SUCCESS:".encode('utf-8').decode('utf-8')+r.content.decode('utf-8', errors='ignore'),end='')
    
elif args.type.lower() == "post":
    if params is None:
        params = {}
    try:
        r = requests.post(args.url,headers = headers, json = params, timeout=args.timeout)
        r.encoding = 'utf-8'
        print("SUCCESS:".encode('utf-8').decode('utf-8')+r.content.decode('utf-8', errors='ignore'),end='')
    except:
        print(f"ERROR 网络异常")
    
else:
    print("ERROR:暂不支持")
    exit(0)
