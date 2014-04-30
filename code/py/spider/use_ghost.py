from ghost import Ghost
ghost = Ghost()
page, extra_resources = ghost.open("http://product.suning.com/102461597.html")
assert page.http_status==200
print '#######'
#print ghost.content.encode("utf-8")
print dir(page.content)

ghost.exit()
