from django.db import models
from django import forms
from django.conf import settings
#from django.core.files import ContentFile
from django.contrib.admin import widgets 
from django.contrib import admin
from django.template.defaultfilters import slugify
from datetime import date
import Image

class MyImage(models.Model):
    image = models.ImageField(upload_to='gallery/')
    thumb = models.ImageField(upload_to='gallery/', blank=True, null=True, editable=False)
    thumbsize = (300,300)
    max_size = 1200
    
    class Meta:
        ordering = ['image']
        
    def thumb(self):
        return '%s' % self.image.url.replace('media/gallery/', 'media/gallery/thumb_')
    
    def __unicode__(self):
        return '%s'  % self.image
    
    def saveThumb(self):
        if self.image is not None:
            path = '%s/%s' % (settings.MEDIA_ROOT, self.image)
            img = Image.open(path)
            width = img.size
            height = width[1]
            width = width[0]
            print '%s x %s' % (width, height)
            if width > self.max_size or height > self.max_size:
                if width > height:
                    tw = self.max_size
                    th = height * self.max_size
                    if width > 0:
                            th = th / width
                else:
                    th = self.max_size
                    tw = width * self.max_size
                    if height > 0:
                            tw = tw / height
                print '%s x %s' % (tw,th)
                img = img.resize((tw, th), Image.ANTIALIAS)
                img.save(path)
            width = img.size
            height = width[1]
            width = width[0]
            if width > height:
                tw = self.thumbsize[0]
                th = height * self.thumbsize[0]
                if width > 0:
                    th = th / width
                img = img.resize((tw, th), Image.ANTIALIAS)
            else:
                th = self.thumbsize[1]
                tw = width * self.thumbsize[1]
                if height > 0:
                    tw = tw / height
                img = img.resize((tw, th), Image.ANTIALIAS)
            path = path.replace('media/gallery/', 'media/gallery/thumb_')
            img.save(path)
            self.thumb = path
    
    def save(self, *args, **kwargs):
        super(MyImage, self).save(*args, **kwargs)
        self.saveThumb()
        
def update_thumbs(modeladmin, request, queryset):
    print queryset
    for query in queryset:
        query.saveThumb()
update_thumbs.short_description = "Resave the thumbnails"

        
class MyImageAdmin(admin.ModelAdmin):
    actions = [update_thumbs]
    class Meta:
        model = MyImage


class Piece(models.Model):
    title = models.CharField(max_length=400)
    default_image = models.ForeignKey(MyImage, null=True, blank=True, related_name='%(app_label)s_%(class)s_default_image')
    slug=models.SlugField(max_length=160,blank=True,editable=False)
    description = models.TextField(null=True, blank=True)
    name = models.CharField(max_length=400)
    materials = models.TextField(null=True, blank=True)
    year = models.IntegerField(null=True,blank=True)
    
    def __unicode__(self):
        return '%s' % (self.title)

    def save(self, *args, **kwargs):
        self.slug = slugify(self.title)
        super(Piece, self).save(*args, **kwargs)
        
class PieceAdmin(admin.ModelAdmin):
    class Meta:
        model = Piece

class PieceForm(forms.ModelForm):
    default_image = forms.ImageField(widget=forms.FileInput(), required=False)
    class Meta:
        model = Piece

admin.site.register(MyImage, MyImageAdmin)
admin.site.register(Piece, PieceAdmin)
