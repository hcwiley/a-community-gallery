from django.conf.urls.defaults import *
from django.contrib import admin 
from django.views.generic.simple import redirect_to, direct_to_template
from acg.settings import DEBUG, MEDIA_ROOT
from acg.settings import AJAX_VIEW_PREFIX as ajax
import acg.settings as settings
admin.autodiscover()

# custom 404 and 500 handlers
handler404 = 'views.django'
handler500 = 'views.django_500'

# basic stuff
urlpatterns = patterns('',
    (r'^favicon.ico$', redirect_to, {'url': '/site_media/static/images/fav.ico'}),
    (r'^admin/', include(admin.site.urls)),
    (r'^admin', include(admin.site.urls)),
#    (r'^admin/doc/', include('django.contrib.admindocs.urls')),
    (r'^robots.txt$', direct_to_template, {'template':'robots.txt', 'mimetype':'text/plain'}),
    (r'^sitemap.txt$', direct_to_template, {'template':'sitemap.txt', 'mimetype':'text/plain'}),
    (r'^add-work$', 'views.add_work'),
    (r'^xml/gallery$', 'views.xml'),
)

if DEBUG:
#     let us test out missing page and server error when debugging
    urlpatterns += patterns('',
#        (r'^%s(?P<page>.*)$' % 'site_media/media/gallery/' , 'views.ajax'),
        (r'^%s*/(?P<path>.*)$' % 'site_media/media/' , 'django.views.static.serve', {'document_root': settings.STATIC_DOC_ROOT}),
        (r'^404$', 'views.gallery_404'),
        (r'^500$', 'views.gallery_500'),
    )

# public pages w/ajax support
urlpatterns += patterns('',
#    (r'^(?P<ajax>(%s)?)$' % ajax, 'views.home'),
    (r'^%s(?P<page>.*)$' % ajax, 'views.ajax'),
    (r'^(?P<page>.*)$', 'views.default'),
#    (r'^(?P<ajax>(%s)?)contact$' % ajax, 'views.contact'),
)

# oh why oh why isn't there a REMOVE_SLASH option...
urlpatterns += patterns('',
#    (r'^admin$', 'views.admin_add_slash'),
    (r'^(?P<url>.*)$', 'views.remove_slash'),
)

